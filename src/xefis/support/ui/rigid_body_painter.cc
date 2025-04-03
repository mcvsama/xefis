/* vim:ts=4
 *
 * Copyleft 2022  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Local:
#include "rigid_body_painter.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/color/spaces.h>
#include <xefis/support/earth/air/atmospheric_scattering.h>
#include <xefis/support/math/rotations.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/simulation/constraints/fixed_constraint.h>
#include <xefis/support/simulation/constraints/hinge_constraint.h>
#include <xefis/support/simulation/constraints/slider_constraint.h>
#include <xefis/support/simulation/devices/wing.h>
#include <xefis/support/simulation/rigid_body/various_materials.h>
#include <xefis/support/simulation/rigid_body/various_shapes.h>
#include <xefis/support/ui/gl_space.h>
#include <xefis/support/ui/paint_helper.h>

// Neutrino:
#include <neutrino/stdexcept.h>
#include <neutrino/utility.h>

// Qt:
#include <QImage>
#include <QOpenGLFunctions>
#include <QPainter>

// System:
#include <GL/gl.h>
#include <GL/glu.h>

// Standard:
#include <cstddef>
#include <future>
#include <random>


namespace xf {

constexpr auto kGLAtmosphericSunLight	= GL_LIGHT0;
constexpr auto kGLCosmicSunLight		= GL_LIGHT1;
constexpr auto kGLFeatureLight			= GL_LIGHT2;
constexpr auto kGLSkyLight0				= GL_LIGHT3;
constexpr auto kGLSkyLight1				= GL_LIGHT4;
constexpr auto kGLSkyLight2				= GL_LIGHT5;
constexpr auto kGLSkyLight3				= GL_LIGHT6;
constexpr auto kGLSkyLight4				= GL_LIGHT7;

// For debugging:
constexpr auto kBlackSunsEnabled	= false;


RigidBodyPainter::RigidBodyPainter (si::PixelDensity const pixel_density):
	_pixel_density (pixel_density),
	_gl (pixel_density * kDefaultPositionScale)
{
	_sky_lights[0] = SkyLight {
		.gl_number	= kGLSkyLight0,
		.position	= { 0_deg, 0_deg },
	};
	_sky_lights[1] = SkyLight {
		.gl_number	= kGLSkyLight1,
		.position	= { 90_deg, 5_deg },
	};
	_sky_lights[2] = SkyLight {
		.gl_number	= kGLSkyLight2,
		.position	= { 180_deg, 5_deg },
	};
	_sky_lights[3] = SkyLight {
		.gl_number	= kGLSkyLight3,
		.position	= { 270_deg, 5_deg },
	};
	_sky_lights[4] = {
		.gl_number	= kGLSkyLight4,
		.position	= { 0_deg, 90_deg },
	};

	calculate_camera_transform();
	_sky_dome = calculate_sky_dome();
}


void
RigidBodyPainter::set_time (si::Time const time)
{
	if (abs (time - _sky_dome_update_time) > 1_s)
	{
		_need_new_sky_dome = true;
		_sky_dome_update_time = time;
	}

	_time = time;
}


void
RigidBodyPainter::set_followed_to_none()
{
	_followed = std::monostate();
	calculate_camera_transform();
}


void
RigidBodyPainter::set_camera_mode (CameraMode const mode)
{
	_camera_mode = mode;
	calculate_camera_transform();
}


void
RigidBodyPainter::set_camera_position (si::LonLatRadius<> const position)
{
	_requested_camera_polar_position = position;
}


void
RigidBodyPainter::set_user_camera_translation (SpaceLength<WorldSpace> const& translation)
{
	_user_camera_translation = translation;
	calculate_camera_transform();
}


void
RigidBodyPainter::set_user_camera_rotation (SpaceVector<si::Angle> const& angles)
{
	if (_user_camera_angles != angles)
	{
		_user_camera_angles = angles;
		auto const x = x_rotation<WorldSpace> (angles.x());
		auto const y = y_rotation<WorldSpace> (angles.y());
		auto const z = z_rotation<WorldSpace> (angles.z());
		_user_camera_rotation = x * y * z;
		calculate_camera_transform();
	}
}


void
RigidBodyPainter::set_planet (rigid_body::Body const* planet_body)
{
	_planet_body = planet_body;
	calculate_camera_transform();
}


void
RigidBodyPainter::paint (rigid_body::System const& system, QOpenGLPaintDevice& canvas)
{
	_group_centers_of_mass_cache.clear();
	initializeOpenGLFunctions();

	// If loading of Earth texture wasn't started yet:
	if (!_earth_texture && !_next_earth_texture_image.valid())
	{
		auto const load = [] {
			return QImage ("share/images/textures/earth-day-with-clouds.jpg");
		};

		if (_work_performer)
			_next_earth_texture_image = _work_performer->submit (load);
		else
			_next_earth_texture_image = std::async (std::launch::deferred, load);
	}

	auto const ph = PaintHelper (canvas);

	QPainter painter (&canvas);
	ph.setup_painter (painter);

	QRectF rect (0, 0, canvas.width(), canvas.height());
	QPointF center = rect.center();

	painter.translate (center);
	painter.beginNativePainting();
	precalculate();
	setup_camera (canvas);
	setup_lights();
	paint_world (system);
	paint_ecef_basis (canvas);
	painter.endNativePainting();
}


void
RigidBodyPainter::precalculate()
{
	_followed_polar_position = to_polar (math::coordinate_system_cast<ECEFSpace, void> (followed_position() - planet_position()));
	check_earth_texture();
	check_sky_dome();
}


void
RigidBodyPainter::setup_camera (QOpenGLPaintDevice& canvas)
{
	setup_camera_projection (canvas.size());
	setup_modelview();
	calculate_camera_transform();
}


void
RigidBodyPainter::setup_camera_projection (QSize const size)
{
	glMatrixMode (GL_PROJECTION);
	_gl.load_identity();
	_gl.translate (0.0, 0.0, -1.0);
	_gl.set_hfov_perspective (size, 40_deg, _gl.to_opengl (1_m), _gl.to_opengl (100_km));
}


void
RigidBodyPainter::setup_modelview()
{
	glMatrixMode (GL_MODELVIEW);
	glFrontFace (GL_CCW);
	glCullFace (GL_BACK);
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glPolygonMode (GL_BACK, GL_LINE);
	glShadeModel (GL_SMOOTH);
	glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_CULL_FACE);
	glDepthFunc (GL_LEQUAL);
	glDepthMask (GL_TRUE);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable (GL_LIGHTING);
	_gl.load_identity();
}


void
RigidBodyPainter::setup_lights()
{
	setup_sun_light();
	setup_sky_light();
	setup_feature_light();

	// Disable all lights by default:
	enable_only_lights (0);
}


void
RigidBodyPainter::setup_sun_light()
{
	// Blend the original sun color with color as seen through the atmosphere:
	auto const q_atmospheric_sun_color = QColor::fromRgbF (_sky_dome.sun_light_color[0], _sky_dome.sun_light_color[1], _sky_dome.sun_light_color[2]);
	auto const x = neutrino::renormalize (_followed_polar_position.radius(), Range { kEarthMeanRadius, kAtmosphereRadius }, Range { 0.0f, 1.0f });
	auto const atmospheric_sun_color = to_gl_color (hsl_interpolation (x, q_atmospheric_sun_color, _sun_color_in_space));
	auto const cosmic_sun_color = to_gl_color (_sun_color_in_space);

	glLightfv (kGLAtmosphericSunLight, GL_AMBIENT, atmospheric_sun_color.scaled (0.0f));
	glLightfv (kGLAtmosphericSunLight, GL_DIFFUSE, atmospheric_sun_color.scaled (0.4f));
	glLightfv (kGLAtmosphericSunLight, GL_SPECULAR, atmospheric_sun_color.scaled (0.1f));

	glLightfv (kGLCosmicSunLight, GL_AMBIENT, cosmic_sun_color.scaled (0.0f));
	glLightfv (kGLCosmicSunLight, GL_DIFFUSE, cosmic_sun_color.scaled (0.4f));
	glLightfv (kGLCosmicSunLight, GL_SPECULAR, cosmic_sun_color.scaled (0.1f));

	_gl.save_context ([&] {
		_gl.set_camera (_camera);

		if (_planet_body)
		{
			_gl.save_context ([&] {
				_gl.load_identity();
				_gl.set_camera_rotation_only (_camera);
				make_z_towards_the_sun();
				// Direct atmospheric sunlight:
				glLightfv (kGLAtmosphericSunLight, GL_POSITION, GLArray { 0.0f, 0.0f, _gl.to_opengl (kSunDistance), 1.0f });
				// Direct cosmic sunlight:
				glLightfv (kGLCosmicSunLight, GL_POSITION, GLArray { 0.0f, 0.0f, _gl.to_opengl (kSunDistance), 1.0f });
			});
		}
		else
		{
			// Otherwise let the observer cast the light:
			glLightfv (kGLAtmosphericSunLight, GL_POSITION, GLArray { 0.0f, 0.0f, 1.0f, 0.0f });
			glLightfv (kGLCosmicSunLight, GL_POSITION, GLArray { 0.0f, 0.0f, 1.0f, 0.0f });
		}
	});

	// TODO GL_SPOT_DIRECTION
	// TODO GL_SPOT_EXPONENT
	// TODO GL_SPOT_CUTOFF
}


void
RigidBodyPainter::set_atmospheric_sun_light_enabled (bool enabled)
{
	if (enabled)
		glEnable (kGLAtmosphericSunLight);
	else
		glDisable (kGLAtmosphericSunLight);
}


void
RigidBodyPainter::set_cosmic_sun_light_enabled (bool enabled)
{
	if (enabled)
		glEnable (kGLCosmicSunLight);
	else
		glDisable (kGLCosmicSunLight);
}


void
RigidBodyPainter::setup_sky_light()
{
	// Sky lights:
	for (auto& sky_light: _sky_lights)
	{
		_gl.save_context ([&] {
			make_z_sky_top_x_sun_azimuth();
			_gl.rotate_z (+sky_light.position.lon());
			_gl.rotate_y (-sky_light.position.lat());

			auto const number = sky_light.gl_number;
			auto const light_direction = to_cartesian<void> (sky_light.position); // Azimuth (lon()) should possible be negated, but the sky dome is symmetric, so this is okay.
			auto const color = _atmospheric_scattering.calculate_incident_light (
				{ 0_m, 0_m, _followed_polar_position.radius() },
				light_direction,
				_sky_dome.sun_position.cartesian_coordinates
			);
			auto const corrected_color = sky_correction (color);
			auto const gl_color = to_gl_color (corrected_color);
			auto const sky_height = kAtmosphereRadius - kEarthMeanRadius;

			glEnable (number);
			glLightfv (number, GL_AMBIENT, gl_color.scaled (0.0f));
			glLightfv (number, GL_DIFFUSE, gl_color.scaled (0.2f));
			glLightfv (number, GL_SPECULAR, gl_color.scaled (0.0f));
			glLightfv (number, GL_POSITION, GLArray { _gl.to_opengl (sky_height), 0.0f, 0.0f, 0.0f });
		});
	}
}


void
RigidBodyPainter::set_sky_light_enabled (bool enabled)
{
	for (auto& sky_light: _sky_lights)
	{
		if (enabled)
			glEnable (sky_light.gl_number);
		else
			glDisable (sky_light.gl_number);
	}
}


void
RigidBodyPainter::setup_feature_light()
{
	// Enable feature light:
	glLightfv (kGLFeatureLight, GL_AMBIENT, GLArray { 0.25f, 0.25f, 0.25f, 1.0f });
	glLightfv (kGLFeatureLight, GL_DIFFUSE, GLArray { 0.5f, 0.5f, 0.5f, 1.0f });
	glLightfv (kGLFeatureLight, GL_SPECULAR, GLArray { 0.9f, 0.9f, 0.9f, 1.0f });

	_gl.save_context ([&] {
		// Reset rotations and translations:
		_gl.load_identity();
		// Cast light from observer's position (Z = 1):
		glLightfv (kGLFeatureLight, GL_POSITION, GLArray { 0.0f, 0.0f, 1.0f, 0.0f });
	});
}


void
RigidBodyPainter::set_feature_light_enabled (bool enabled)
{
	if (enabled)
		glEnable (kGLFeatureLight);
	else
		glDisable (kGLFeatureLight);
}


void
RigidBodyPainter::enable_only_lights (uint32_t lights)
{
	if (lights > 0)
	{
		glEnable (GL_LIGHTING);
		set_atmospheric_sun_light_enabled (lights & kAtmosphericSunLight);
		set_cosmic_sun_light_enabled (lights & kCosmicSunLight);
		set_sky_light_enabled (lights & kSkyLight);
		set_feature_light_enabled (lights & kFeatureLight);
	}
	else
		glDisable (GL_LIGHTING);
}


void
RigidBodyPainter::make_z_towards_the_sun()
{
	// Assuming we have identity rotations + camera rotations applied.

	// Rotate sun depending on UTC time:
	auto const greenwich_hour_angle = _sky_dome.sun_position.hour_angle - _camera_polar_position.lon();
	_gl.rotate_z (-greenwich_hour_angle);

	// Rotate sun depending on time of the year:
	_gl.rotate_y (-_sky_dome.sun_position.declination);

	// Our sun is located over the North Pole (+Z in ECEF). Rotate it to be straight over Null Island
	// that is lon/lat 0°/0° (+X in ECEF):
	_gl.rotate_y (90_deg);
}


void
RigidBodyPainter::make_z_sky_top_x_south()
{
	_gl.rotate (~(z_rotation<WorldSpace> (_camera_polar_position.lon()) * y_rotation<WorldSpace> (-_camera_polar_position.lat() + 90_deg)));
	// Z is now sky top, X is towards azimuth 180° (true south).
}


void
RigidBodyPainter::make_z_sky_top_x_sun_azimuth()
{
	make_z_sky_top_x_south();
	_gl.rotate_z (-_sky_dome.sun_position.horizontal_coordinates.azimuth + 180_deg);
}


void
RigidBodyPainter::paint_world (rigid_body::System const& system)
{
	_gl.save_context ([&] {
		paint_planet();
		paint_air_particles();
		paint (system);
	});
}


void
RigidBodyPainter::paint_planet()
{
	if (!_planet_body)
		return; // TODO at least draw then sun with shines

	_gl.save_context ([&] {
		_gl.set_camera (_camera);

		// Sun:
		_gl.save_context ([&] {
			// TODO to func: get_sun_face_shape(); recalculated occassionally
			auto sun_face_material = rigid_body::kWhiteMatte;
			sun_face_material.gl_emission_color = GLColor (1.0, 1.0, 1.0);
			auto enlargement = neutrino::renormalize (_sky_dome.sun_position.horizontal_coordinates.altitude, Range { 0_deg, 90_deg }, Range { kSunSunsetEnlargement, kSunNoonEnlargement });
			auto sun_face = rigid_body::make_solid_circle (enlargement * kSunRadius, { 0_deg, 360_deg }, 19, sun_face_material);

			// Disable Z-testing so that the sun gets rendered even if it's far behind the sky dome sphere:
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable (GL_ALPHA_TEST);

			// Enable blending, to blend well the Sun with the sky:
			glDisable (GL_DEPTH_TEST);
			glDisable (GL_LIGHTING);
			glEnable (GL_BLEND);
			// Same remark as for sky rendering about what's considered the front face here:
			glFrontFace (GL_CW);

			_gl.save_context ([&] {
				make_z_towards_the_sun();
				// Z is now direction towards the Sun:
				_gl.translate (0_m, 0_m, kSunDistance);
				_gl.draw (sun_face);
			});

			if constexpr (kBlackSunsEnabled)
			{
				// For debugging.
				// Draw black balls at the positions of additional source of lights.

				auto black_sun = rigid_body::make_centered_sphere_shape ({
					.radius = 30_m,
					.n_slices = 10,
					.n_stacks = 10,
					.material = rigid_body::kBlackMatte,
				});

				for (auto& sky_light: _sky_lights)
				{
					_gl.save_context ([&] {
						make_z_sky_top_x_sun_azimuth();
						_gl.save_context ([&] {
							_gl.rotate_z (+sky_light.position.lon());
							_gl.rotate_y (-sky_light.position.lat());
							_gl.translate (1_km, 0_m, 0_m);
							_gl.draw (black_sun);
						});
					});
				}
			}
		});

		// Ground:
		// TODO it would be best if there was a shader that adds the dome sphere color to the drawn feature
		_gl.save_context ([&] {
			_gl.set_camera (_camera);
			_gl.translate (planet_position());
			_gl.translate (-_camera.position());
			make_z_sky_top_x_south();
			_gl.translate (_camera.position());

			glFrontFace (GL_CCW);
			glDisable (GL_BLEND);
			glEnable (GL_DEPTH_TEST);
			glEnable (GL_TEXTURE_2D);
			enable_only_lights (kCosmicSunLight);
			_gl.draw (_ground_shape);
			glDisable (GL_TEXTURE_2D);
		});

		// Sky and ground dome around the observer:
		_gl.save_context ([&] {
            _gl.set_camera_rotation_only (_camera);
			// Rotate so that down is -Z.
			make_z_sky_top_x_south();
			// Normally the outside of the sphere shape is rendered, inside is culled.
			// But here we're inside the sphere, so tell OpenGL that the front faces are the
			// inside faces:
			glFrontFace (GL_CW);
			// No lights, the sky itself is the light source:
			glDisable (GL_LIGHTING);
			glDisable (GL_DEPTH_TEST);
			// Blend with the universe: final_color = (1 - transmittance) * atmosphere_color + universe_color
			glBlendFunc (GL_SRC_ALPHA, GL_ONE);
			glEnable (GL_BLEND);
			_gl.draw (_sky_dome.atmospheric_dome_shape);
			glDisable (GL_BLEND);
			glEnable (GL_LIGHTING);
			glEnable (GL_DEPTH_TEST);
			glFrontFace (GL_CCW);
		});
	});
}


void
RigidBodyPainter::paint_air_particles()
{
	_gl.save_context ([&] {
		enable_only_lights (kAtmosphericSunLight | kSkyLight);
		_gl.set_camera_rotation_only (_camera);
		// Trick with rotating camera and then subtracting camera position from the object is to avoid problems with low precision OpenGL floats:
		// followed_position() - _camera.position() uses doubles; but _gl.translate() internally reduces them to floats:
		_gl.translate (followed_position() - _camera.position());

		// Air 'particles' only appear if we have a planet:
		auto const dust_size = 2_cm;
		auto const grid_size = 5_m;
		auto dust_material = rigid_body::kWhiteMatte;
		dust_material.set_emission_color (Qt::white);
		auto const dust = rigid_body::make_centered_sphere_shape ({ .radius = dust_size, .n_slices = 3, .n_stacks = 3, .material = dust_material });
		auto const range = 3 * grid_size;

		// Figure out nearest 3D grid points.
		// Then wiggle each one pseudo-randomly.
		auto const body_pos = followed_position();
		auto const inv_grid_size = 1 / grid_size;
		auto const rounded_to_grid = body_pos.transformed ([inv_grid_size, grid_size] (auto const value) {
			return std::round (value * inv_grid_size) * grid_size;
		});
		auto const prng_factor = grid_size / _air_particles_prng.max();
		auto const half_grid_size = 0.5 * grid_size;

		// Paint a grid of little dust particles around the camera position:
		for (auto x = rounded_to_grid.x() - range; x <= rounded_to_grid.x() + range; x += grid_size)
		{
			for (auto y = rounded_to_grid.y() - range; y <= rounded_to_grid.y() + range; y += grid_size)
			{
				for (auto z = rounded_to_grid.z() - range; z <= rounded_to_grid.z() + range; z += grid_size)
				{
					// (x + y + z) makes adjacent points to be wiggled the same amount, so multiply other axes by some big numbers:
					_air_particles_prng.seed ((x + 10 * y + 100 * z).in<si::Meter>());
					_gl.save_context ([&] {
						// OpenGL's center of the view [0, 0, 0] is at body_pos, hence subtracting body_pos from
						// absolute values (in WorldOrigin space) x, y, z here:
						auto const wiggled_x = x - body_pos.x() + prng_factor * _air_particles_prng() - half_grid_size;
						auto const wiggled_y = y - body_pos.y() + prng_factor * _air_particles_prng() - half_grid_size;
						auto const wiggled_z = z - body_pos.z() + prng_factor * _air_particles_prng() - half_grid_size;
						_gl.translate (wiggled_x, wiggled_y, wiggled_z);
						_gl.draw (dust);
					});
				}
			}
		}
	});
}


void
RigidBodyPainter::paint (rigid_body::System const& system)
{
	_gl.save_context ([&] {
		enable_only_lights (kAtmosphericSunLight | kSkyLight);
		_gl.set_camera_rotation_only (_camera);

		for (auto const& body: system.bodies())
			paint (*body, get_rendering_config (*body));

		if (constraints_visible())
			for (auto const& constraint: system.constraints())
				paint (*constraint);

		if (gravity_visible() || aerodynamic_forces_visible() || external_forces_visible())
			for (auto const& body: system.bodies())
				paint_forces (*body);

		if (angular_velocities_visible())
			for (auto const& body: system.bodies())
				paint_angular_velocity (*body);

		if (angular_momenta_visible())
			for (auto const& body: system.bodies())
				paint_angular_momentum (*body);

		// We'll now paint features that are always visible, so clear the Z buffer
		// in OpenGL and do the painting with blending enabled.
		{
			auto const* focused_group = this->focused_group();
			auto const* focused_body = this->focused_body();

			glEnable (GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			_gl.clear_z_buffer();
			enable_only_lights (kFeatureLight);

			for (auto const& group: system.groups())
				paint_helpers (*group, get_rendering_config (*group), group.get() == focused_group);

			for (auto const& body: system.bodies())
				paint_helpers (*body, get_rendering_config (*body), body.get() == focused_body);

			glDisable (GL_BLEND);
		}
	});
}


void
RigidBodyPainter::transform_gl_to_center_of_mass (rigid_body::Group const& group)
{
	// Transform so that center-of-mass is at the OpenGL space origin:
	auto const* rotation_reference_body = group.rotation_reference_body();
	auto const rotation = rotation_reference_body
		? rotation_reference_body->placement().body_to_base_rotation()
		: kNoRotation<WorldSpace, BodyCOM>;
	_gl.transform (Placement<WorldSpace, BodyCOM> (get_center_of_mass (group), rotation) - _camera.position());
}


void
RigidBodyPainter::transform_gl_to_center_of_mass (rigid_body::Body const& body)
{
	// Transform so that center-of-mass is at the OpenGL space origin.
	// Trick with rotating camera and then subtracting camera position from the object is to avoid problems with low precision OpenGL floats:
	// body.placement() - _camera.position() uses doubles; but _gl.transform() internally reduces them to floats:
	_gl.transform (body.placement() - _camera.position());
}


void
RigidBodyPainter::transform_gl_from_body_center_of_mass_to_origin (rigid_body::Body const& body)
{
	_gl.transform (body.origin_placement());
}


void
RigidBodyPainter::paint (rigid_body::Body const& body, BodyRenderingConfig const& rendering)
{
	_gl.save_context ([&] {
		transform_gl_to_center_of_mass (body);

		if (rendering.moments_of_inertia_visible)
			paint_moments_of_inertia_cuboid (body.mass_moments<BodyCOM>());

		// Body shapes are defined relative to BodyOrigin coordinates, so transform again:
		transform_gl_from_body_center_of_mass_to_origin (body);

		if (rendering.body_visible)
		{
			_gl.save_context ([&] {
				if (&body == focused_body())
					_gl.additional_parameters().color_override = GLColor::from_rgb (0x00, 0xaa, 0x7f);
				else if (hovered<rigid_body::Body>() == &body)
					_gl.additional_parameters().color_override = GLColor::from_rgb (0x00, 0xaa, 0x7f).lighter (0.5);

				if (auto const& shape = body.shape())
					_gl.draw (*shape);
				else
					_gl.draw (rigid_body::make_centered_cube_shape (body.mass_moments<BodyCOM>()));
			});
		}
	});
}


void
RigidBodyPainter::paint (rigid_body::Constraint const& constraint)
{
	if (constraint.enabled() && !constraint.broken())
	{
		_gl.save_context ([&] {
			auto const cp = _camera.position();
			auto const& b1 = constraint.body_1();
			auto const& b2 = constraint.body_2();
			auto com1 = b1.placement().position() - cp;
			auto com2 = b2.placement().position() - cp;

			auto const rod_from_to = [this] (si::Length const radius, auto const& from, auto const& to, bool front_back_faces, rigid_body::ShapeMaterial const& material)
			{
				_gl.save_context ([&] {
					auto const diff = to - from;
					_gl.translate (from);

					auto const alpha_beta = alpha_beta_from_x_to (diff);
					_gl.rotate_z (alpha_beta[0]);
					_gl.rotate_y (alpha_beta[1] + 90_deg);

					auto const shape = rigid_body::make_cylinder_shape ({
						.length = abs (diff),
						.radius = radius,
						.num_faces = 16,
						.with_bottom = front_back_faces,
						.with_top = front_back_faces,
						.material = material,
					});
					_gl.draw (shape);
				});
			};

			if (auto const* hinge = dynamic_cast<rigid_body::HingeConstraint const*> (&constraint))
			{
				auto const a1 = b1.placement().unbound_transform_to_base (hinge->hinge_precalculation().body_1_anchor());
				auto const hinge_1 = b1.placement().unbound_transform_to_base (hinge->hinge_precalculation().body_1_hinge());
				auto const hinge_start_1 = com1 + a1;
				auto const hinge_end_1 = hinge_start_1 + hinge_1;
				auto const hinge_center = hinge_start_1 + 0.5 * hinge_1;
				auto const color = &constraint == focused_constraint()
					? QColor (0x00, 0xaa, 0x7f)
					: QColor (0xff, 0x99, 0x00);
				auto const material = rigid_body::make_material (color);

				// Lines from COM to hinge center:
				rod_from_to (kDefaultConstraintDiameter, com1, hinge_center, false, material);
				rod_from_to (kDefaultConstraintDiameter, com2, hinge_center, false, material);
				// Hinge:
				rod_from_to (kDefaultHingeDiameter, hinge_start_1, hinge_end_1, true, material);
			}
			else if (dynamic_cast<rigid_body::FixedConstraint const*> (&constraint))
			{
				auto const color = &constraint == focused_constraint()
					? QColor (0x00, 0xaa, 0x7f)
					: QColor (0xff, 0x00, 0x99);
				auto const material = rigid_body::make_material (color);
				rod_from_to (kDefaultConstraintDiameter, com1, com2, false, material);
			}
		});
	}
}


void
RigidBodyPainter::paint_helpers (rigid_body::Group const& group, GroupRenderingConfig const& rendering, bool focused)
{
	if (focused || rendering.center_of_mass_visible)
	{
		_gl.save_context ([&] {
			transform_gl_to_center_of_mass (group);
			paint_center_of_mass();

			if (group.rotation_reference_body())
				paint_basis (_user_camera_translation.norm() / 20); // Rotation was taken into account in transform_gl_to_center_of_mass (Group).
		});
	}
}


void
RigidBodyPainter::paint_helpers (rigid_body::Body const& body, BodyRenderingConfig const& rendering, bool focused)
{
	if (focused || rendering.center_of_mass_visible || rendering.origin_visible)
	{
		_gl.save_context ([&] {
			transform_gl_to_center_of_mass (body);

			if (focused || rendering.center_of_mass_visible)
				paint_center_of_mass();

			if (focused || rendering.origin_visible)
			{
				transform_gl_from_body_center_of_mass_to_origin (body);

				// Paint the yellow origin ball only if explicitly requested,
				// otherwise the user will know that the origin is where the basis is:
				if (rendering.origin_visible)
					paint_origin();

				paint_basis (_user_camera_translation.norm() / 20);
			}
		});
	}
}


void
RigidBodyPainter::paint_center_of_mass()
{
	auto const com_shape = rigid_body::make_center_of_mass_symbol_shape (_user_camera_translation.norm() / 150);

	_gl.save_context ([&] {
		glDisable (GL_LIGHTING);
		_gl.draw (com_shape);
		glEnable (GL_LIGHTING);
	});
}


void
RigidBodyPainter::paint_origin()
{
	auto const origin_material = rigid_body::make_material ({ 0xff, 0xff, 0x00 });
	auto const origin_shape = rigid_body::make_centered_sphere_shape ({ .radius = _user_camera_translation.norm() / 150, .n_slices = 8, .n_stacks = 4, .material = origin_material });
	_gl.draw (origin_shape);
}


void
RigidBodyPainter::paint_moments_of_inertia_cuboid (MassMoments<BodyCOM> const& mass_moments)
{
	auto const com_material = rigid_body::make_material ({ 0x00, 0x44, 0x99 });
	auto const com_shape = make_centered_cube_shape (mass_moments, com_material);
	_gl.draw (com_shape);
}


void
RigidBodyPainter::paint_forces (rigid_body::Body const& body)
{
	auto constexpr gravity_color = Qt::magenta;
	auto constexpr lift_color = Qt::green;
	auto constexpr drag_color = Qt::red;
	auto constexpr torque_color = Qt::blue;
	auto constexpr external_force_color = Qt::darkYellow;
	auto constexpr external_torque_color = Qt::cyan;

	auto constexpr force_to_length = 0.1_m / 1_N; // TODO unhardcode; make autoscaling depending on aircraft total mass
	auto constexpr torque_to_length = force_to_length / 1_m; // TODO unhardcode

	auto const& iter = body.iteration();
	auto const gfm = iter.gravitational_force_moments;
	auto const efm = iter.external_force_moments;
	auto const cp = _camera.position();
	auto const com = body.placement().position() - cp;

	if (_gravity_visible)
		draw_arrow (com, gfm.force() * force_to_length, rigid_body::make_material (gravity_color));

	if (_aerodynamic_forces_visible)
	{
		if (auto const* wing = dynamic_cast<sim::Wing const*> (&body))
		{
			if (auto const params = wing->airfoil_aerodynamic_parameters();
				params)
			{
				auto const& forces = params->forces;
				auto const& pl = wing->placement();
				auto const at = pl.bound_transform_to_base (forces.center_of_pressure) - cp;

				draw_arrow (at, pl.unbound_transform_to_base (forces.lift) * force_to_length, rigid_body::make_material (lift_color));
				draw_arrow (at, pl.unbound_transform_to_base (forces.drag) * force_to_length, rigid_body::make_material (drag_color));
				draw_arrow (at, pl.unbound_transform_to_base (forces.pitching_moment) * torque_to_length, rigid_body::make_material (torque_color));
			}
		}
	}

	if (_external_forces_visible)
	{
		draw_arrow (com, efm.force() * force_to_length, rigid_body::make_material (external_force_color));
		draw_arrow (com, efm.torque() * torque_to_length, rigid_body::make_material (external_torque_color));
	}
}


void
RigidBodyPainter::paint_angular_velocity (rigid_body::Body const& body)
{
	auto constexpr angular_velocity_to_length = 0.1_m / 1_radps; // TODO unhardcode
	auto const com = body.placement().position() - _camera.position();
	auto const omega = body.velocity_moments<WorldSpace>().angular_velocity();

	draw_arrow (com, omega * angular_velocity_to_length, rigid_body::make_material (Qt::darkMagenta));
}


void
RigidBodyPainter::paint_angular_momentum (rigid_body::Body const& body)
{
	auto constexpr angular_momentum_to_length = 0.001_m / (1_kg * 1_m2 / 1_s) / 1_rad; // TODO unhardcode
	auto const com = body.placement().position() - _camera.position();
	auto const I = body.mass_moments<BodyCOM>().inertia_tensor();
	auto const L = I * body.velocity_moments<BodyCOM>().angular_velocity();
	auto const L_world = body.placement().unbound_transform_to_base (L);

	draw_arrow (com, L_world * angular_momentum_to_length, rigid_body::make_material (Qt::darkBlue));
}


void
RigidBodyPainter::draw_arrow (SpaceLength<WorldSpace> const& origin, SpaceLength<WorldSpace> const& vector, rigid_body::ShapeMaterial const& material)
{
	_gl.save_context ([&] {
		auto const length = abs (vector);

		if (length > 0_m)
		{
			auto constexpr scale = 2.0f;
			auto constexpr kNumFaces = 12;
			auto constexpr cone_radius = 20_mm * scale;
			auto constexpr cone_length = 50_mm * scale;
			auto constexpr radius = 5_mm * scale;
			auto const alpha_beta = alpha_beta_from_x_to (vector);

			_gl.translate (origin);
			_gl.rotate_z (alpha_beta[0]);
			_gl.rotate_y (alpha_beta[1] + 90_deg);
			_gl.draw (rigid_body::make_cylinder_shape ({ .length = length, .radius = radius, .num_faces = kNumFaces, .with_bottom = true, .with_top = true, .material = material }));
			_gl.translate (0_m, 0_m, length);
			_gl.draw (rigid_body::make_cone_shape ({ .length = cone_length, .radius = cone_radius, .num_faces = kNumFaces, .with_bottom = true, .material = material }));
		}
	});
}


SpaceVector<float, RGBSpace>
RigidBodyPainter::sky_correction (SpaceVector<float, RGBSpace> rgb) const
{
	auto constexpr altitude_threshold = 4_deg;
	auto constexpr reduce_green_to = 0.8;
	auto constexpr reduce_green_to_sqrt = std::sqrt (reduce_green_to);

	if (_sky_dome.sun_position.hour_angle > 0_deg && _sky_dome.sun_position.hour_angle < 180_deg)
	{
		auto const abs_altitude = abs (_sky_dome.sun_position.horizontal_coordinates.altitude);

		if (abs_altitude < altitude_threshold)
		{
			auto const from = Range { altitude_threshold, 0_deg };
			auto const to = Range { 1.0, reduce_green_to_sqrt };
			auto const factor = neutrino::renormalize (_sky_dome.sun_position.horizontal_coordinates.altitude, from, to);
			rgb[1] *= square (factor);
		}
	}

	return rgb;
}


void
RigidBodyPainter::paint_ecef_basis (QOpenGLPaintDevice& canvas)
{
	auto const distance_from_edge = 1.5_cm;
	auto const pixels_from_edge = distance_from_edge * _pixel_density;
	auto const tx = -1.0 + 2 * pixels_from_edge / canvas.width();
	auto const ty = -1.0 + 2 * pixels_from_edge / canvas.height();

	_gl.save_context ([&] {
		glMatrixMode (GL_PROJECTION);
		_gl.load_identity();
		_gl.translate (tx, ty, -1.0);
		_gl.set_hfov_perspective (canvas.size(), 60_deg, _gl.to_opengl (1_cm), _gl.to_opengl (10_m));

		glMatrixMode (GL_MODELVIEW);
		// ECEF basis should be always "on top":
		glClearDepth (1.0f);
		glClear (GL_DEPTH_BUFFER_BIT);
		_gl.set_camera (std::nullopt);
		// TODO Try to use set_camera instead of manually managing the matrix:
		_gl.load_identity();
		_gl.translate (0_m, 0_m, -1_m);
		_gl.rotate (_camera.base_to_body_rotation());
		paint_basis (8_cm);
	});
}


void
RigidBodyPainter::paint_basis (si::Length const length)
{
	si::Length const radius = length / 50;
	si::Length const cone_radius = length / 13;
	si::Length const cone_length = length / 5;

	auto const blue = rigid_body::make_material (QColor (0x11, 0x11, 0xff));
	auto const red = rigid_body::make_material (Qt::red);
	auto const green = rigid_body::make_material (Qt::green);
	auto const kNumFaces = 12;

	setup_feature_light();

	// Root ball:
	_gl.draw (rigid_body::make_centered_sphere_shape ({ .radius = 2 * radius, .n_slices = 8, .n_stacks = 4 }));
	// X axis:
	_gl.save_context ([&] {
		_gl.rotate_y (+90_deg);
		_gl.draw (rigid_body::make_cylinder_shape ({ .length = length, .radius = radius, .num_faces = kNumFaces, .material = red }));
		_gl.translate (0_m, 0_m, length);
		_gl.draw (rigid_body::make_cone_shape ({ .length = cone_length, .radius = cone_radius, .num_faces = kNumFaces, .with_bottom = true, .material = red }));
	});
	// Y axis:
	_gl.save_context ([&] {
		_gl.rotate_x (-90_deg);
		_gl.draw (rigid_body::make_cylinder_shape ({ .length = length, .radius = radius, .num_faces = kNumFaces, .material = green }));
		_gl.translate (0_m, 0_m, length);
		_gl.draw (rigid_body::make_cone_shape ({ .length = cone_length, .radius = cone_radius, .num_faces = kNumFaces, .with_bottom = true, .material = green }));
	});
	// Z axis:
	_gl.save_context ([&] {
		_gl.draw (rigid_body::make_cylinder_shape ({ .length = length, .radius = radius, .num_faces = kNumFaces, .material = blue }));
		_gl.translate (0_m, 0_m, length);
		_gl.draw (rigid_body::make_cone_shape ({ .length = cone_length, .radius = cone_radius, .num_faces = kNumFaces, .with_bottom = true, .material = blue }));
	});
}


SpaceLength<WorldSpace>
RigidBodyPainter::followed_position()
{
	if (auto const* followed_body = this->followed_body())
		return followed_body->placement().position();
	else if (auto const* followed_group = this->followed_group())
		return get_center_of_mass (*followed_group);
	else
		return { 0_m, 0_m, 0_m };
}


SpaceLength<WorldSpace>
RigidBodyPainter::planet_position() const
{
	if (_planet_body)
		return _planet_body->placement().position();
	else
		return SpaceLength<WorldSpace> (math::zero);
}


SpaceLength<WorldSpace>
RigidBodyPainter::get_center_of_mass (rigid_body::Group const& group)
{
	if (auto const it = _group_centers_of_mass_cache.find (&group);
		it != _group_centers_of_mass_cache.end())
	{
		return it->second;
	}
	else
		return _group_centers_of_mass_cache[&group] = group.mass_moments().center_of_mass_position();
}


void
RigidBodyPainter::check_earth_texture()
{
	if (_next_earth_texture_image.valid() && is_ready (_next_earth_texture_image))
	{
		_earth_texture = std::make_shared<QOpenGLTexture> (QOpenGLTexture::Target2D);
		_earth_texture->setData (_next_earth_texture_image.get());
		_earth_texture->setWrapMode (QOpenGLTexture::Repeat);
		_earth_texture->setMinificationFilter (QOpenGLTexture::Linear);
		_earth_texture->setMagnificationFilter (QOpenGLTexture::Linear);
		// Reload SkyDome to include the Earth texture:
		_need_new_sky_dome = true;
	}
}


void
RigidBodyPainter::check_sky_dome()
{
	// If the next calculated SkyDome is ready, use it:
	if (_next_sky_dome.valid() && is_ready (_next_sky_dome))
		_sky_dome = _next_sky_dome.get();

	if (!_next_sky_dome.valid() && std::exchange (_need_new_sky_dome, false))
	{
		if (_work_performer)
			_next_sky_dome = _work_performer->submit (&RigidBodyPainter::calculate_sky_dome, this);
		else
			_sky_dome = calculate_sky_dome();
	}
}


SkyDome
RigidBodyPainter::calculate_sky_dome()
{
	_camera_position_for_sky_dome = _camera.position();

	return xf::calculate_sky_dome ({
		.atmospheric_scattering = _atmospheric_scattering,
		.observer_position = _camera_polar_position,
		.earth_radius = kEarthMeanRadius,
		.unix_time = _time,
		.earth_texture = _earth_texture,
	}, _work_performer);
	// TODO apply sky_correction to vertices' materials
}


void
RigidBodyPainter::calculate_camera_transform()
{
	auto const* followed_group = this->followed_group();
	auto const* followed_body = this->followed_body();

	switch (_camera_mode)
	{
		case CockpitView:
		{
			auto rotation = RotationQuaternion<WorldSpace> (math::identity);

			if (followed_group)
				if (auto const* rotation_reference_body = followed_group->rotation_reference_body())
					followed_body = rotation_reference_body;

			if (followed_body)
			{
				auto const body_rotation = math::coordinate_system_cast<WorldSpace, WorldSpace> (followed_body->placement().base_to_body_rotation());
				// Make an exception if we're following the planet body: we don't want to use aircraft coordinates
				// for the planet, because it's unnatural:
				if (followed_body == _planet_body)
					rotation = _user_camera_rotation * kScreenToNullIslandRotation * body_rotation;
				else
					rotation = _user_camera_rotation * kAircraftToBehindViewRotation * body_rotation;
			}

			_camera.set_body_to_base_rotation (rotation);
			_camera.set_position (followed_position() - planet_position() + ~rotation * _user_camera_translation);
			_camera_polar_position = to_polar (_camera.position());
		}
		break;

		case ChaseView:
		{
			auto const rotation = _user_camera_rotation * gravity_down_rotation (_followed_polar_position) * kScreenToNullIslandRotation;
			_camera.set_body_to_base_rotation (rotation);
			_camera.set_position (followed_position() - planet_position() + ~rotation * _user_camera_translation);
			_camera_polar_position = to_polar (_camera.position());
		}
		break;

		case RCPilotView:
			// TODO unimplemented yet
			// TODO use _requested_camera_polar_position
			break;

		case FixedView:
			// TODO unimplemented yet
			// TODO use _requested_camera_polar_position
			break;
	}

	if (abs (_camera_position_for_sky_dome - _camera.position()) > 10_m)
		_need_new_sky_dome = true;

	_ground_shape = calculate_ground_shape (_camera_polar_position, kEarthMeanRadius, _earth_texture);
}


QColor
RigidBodyPainter::hsl_interpolation (float x, QColor const& color0, QColor const& color1)
{
	x = std::clamp (x, 0.0f, 1.0f);

	auto const hsv1 = color0.toHsl();
	auto const hsv2 = color1.toHsl();

	auto const h = static_cast<int> (hsv1.hue() + x * (hsv2.hue() - hsv1.hue()));
	auto const s = static_cast<int> (hsv1.saturation() + x * (hsv2.saturation() - hsv1.saturation()));
	auto const l = static_cast<int> (hsv1.lightness() + x * (hsv2.lightness() - hsv1.lightness()));
	auto const a = static_cast<int> (hsv1.alpha() + x * (hsv2.alpha() - hsv1.alpha()));

	return QColor::fromHsl (h, s, l, a);
}

} // namespace xf

