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
#include <random>


namespace xf {

constexpr auto kSunLight		= GL_LIGHT0;
constexpr auto kFeatureLight	= GL_LIGHT1;
constexpr auto kSkyLight0		= GL_LIGHT2;
constexpr auto kSkyLight1		= GL_LIGHT3;
constexpr auto kSkyLight2		= GL_LIGHT4;
constexpr auto kSkyLight3		= GL_LIGHT5;
constexpr auto kSkyLight4		= GL_LIGHT6;


RigidBodyPainter::RigidBodyPainter (si::PixelDensity const pixel_density):
	_pixel_density (pixel_density),
	_gl (pixel_density * kDefaultPositionScale)
{
	_sky_lights[0] = SkyLight {
		.gl_number	= kSkyLight0,
		.position	= { 0_deg, 0_deg },
	};
	_sky_lights[1] = SkyLight {
		.gl_number	= kSkyLight1,
		.position	= { 90_deg, 5_deg },
	};
	_sky_lights[2] = SkyLight {
		.gl_number	= kSkyLight2,
		.position	= { 180_deg, 5_deg },
	};
	_sky_lights[3] = SkyLight {
		.gl_number	= kSkyLight3,
		.position	= { 270_deg, 5_deg },
	};
	_sky_lights[4] = {
		.gl_number	= kSkyLight4,
		.position	= { 0_deg, 90_deg },
	};

	_sky_dome = calculate_sky_dome();
}


void
RigidBodyPainter::set_time (si::Time const time)
{
	if (abs (time - _sky_dome_update_time) > 1_s)
	{
		_recalculate_sky_dome = true;
		_sky_dome_update_time = time;
	}

	_time = time;
}


void
RigidBodyPainter::set_camera_angles (si::Angle const x, si::Angle const y, si::Angle const z)
{
	_camera_angles[0] = x; // Pitch
	_camera_angles[1] = y; // Yaw
	_camera_angles[2] = z; // Roll
}


void
RigidBodyPainter::paint (rigid_body::System const& system, QOpenGLPaintDevice& canvas)
{
	_group_centers_of_mass_cache.clear();

	initializeOpenGLFunctions();
	auto const ph = PaintHelper (canvas);

	QPainter painter (&canvas);
	ph.setup_painter (painter);

	QRectF rect (0, 0, canvas.width(), canvas.height());
	QPointF center = rect.center();

	painter.translate (center);
	painter.beginNativePainting();
	setup (canvas);
	paint_world (system);

	// ECEF basis should be always "on top":
	glClearDepth (1.0f);
	glClear (GL_DEPTH_BUFFER_BIT);
	paint_ecef_basis (canvas);

	painter.endNativePainting();
}


void
RigidBodyPainter::setup (QOpenGLPaintDevice& canvas)
{
	using namespace std::chrono_literals;

	auto const size = canvas.size();

	_position_on_earth = xf::polar (math::coordinate_system_cast<ECEFSpace, void> (followed_position()));
	_agl_height = abs (followed_position() + _camera_position) - kEarthMeanRadius;

	// If the next calculated SkyDome is ready, use it:
	if (_next_sky_dome.valid() && is_ready (_next_sky_dome))
		_sky_dome = _next_sky_dome.get();

	if (!_next_sky_dome.valid() && std::exchange (_recalculate_sky_dome, false))
		schedule_sky_dome_recalculation();

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	glTranslatef (0.0, 0.0, -1.0);
	_gl.set_hfov_perspective (size, 40_deg, _gl.to_opengl (1_m), _gl.to_opengl (100_km));

	glMatrixMode (GL_MODELVIEW);
	glFrontFace (GL_CCW);
	glCullFace (GL_BACK);
	glClearColor (0.1, 0.1, 0.1, 0.0);
	glPolygonMode (GL_BACK, GL_LINE);
	glShadeModel (GL_SMOOTH);
	glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable (GL_NORMALIZE);
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_CULL_FACE);
	glDepthFunc (GL_LEQUAL);
	glDepthMask (GL_TRUE);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable (GL_LIGHTING);

	glLoadIdentity();
}


void
RigidBodyPainter::setup_feature_light()
{
	// Disable natural lights:
	glDisable (kSunLight);

	for (auto& sky_light: _sky_lights)
		glDisable (sky_light.gl_number);

	// Enable feature light:
	glEnable (kFeatureLight);
	glLightfv (kFeatureLight, GL_AMBIENT, GLArray { 0.25f, 0.25f, 0.25f, 1.0f });
	glLightfv (kFeatureLight, GL_DIFFUSE, GLArray { 0.5f, 0.5f, 0.5f, 1.0f });
	glLightfv (kFeatureLight, GL_SPECULAR, GLArray { 0.9f, 0.9f, 0.9f, 1.0f });

	_gl.save_context ([&] {
		// Reset rotations and translations:
		glLoadIdentity();
		// Cast light from observer's position (Z = 1):
		glLightfv (kFeatureLight, GL_POSITION, GLArray { 0.0f, 0.0f, 1.0f, 0.0f });
	});
}


void
RigidBodyPainter::setup_natural_light()
{
	auto const sun_color = to_gl_color (_sky_dome.sun_light_color);
	glDisable (kFeatureLight);
	glEnable (kSunLight);
	glLightfv (kSunLight, GL_AMBIENT, sun_color.scaled (0.2f));
	glLightfv (kSunLight, GL_DIFFUSE, sun_color.scaled (0.4f));
	glLightfv (kSunLight, GL_SPECULAR, sun_color.scaled (0.1f));

	if (_planet_body)
	{
		// Direct sunlight:
		_gl.save_context ([&] {
			glLoadIdentity();
			center_at_followed_object();
			make_z_towards_the_sun();
			glLightfv (kSunLight, GL_POSITION, GLArray { 0.0f, 0.0f, _gl.to_opengl (kSunDistance), 0.0f });
		});

		// Sky lights:
		for (auto& sky_light: _sky_lights)
		{
			_gl.save_context ([&] {
				make_z_sky_top_x_sun_azimuth();
				_gl.rotate_z (+sky_light.position.lon());
				_gl.rotate_y (-sky_light.position.lat());

				auto const number = sky_light.gl_number;
				auto const light_direction = cartesian<void> (sky_light.position); // Azimuth (lon()) should possible be negated, but the sky dome is symmetric, so this is okay.
				auto const color = _atmospheric_scattering.calculate_incident_light (
					{ 0_m, 0_m, _position_on_earth.radius() },
					light_direction,
					_sky_dome.sun_position.cartesian_coordinates
				);
				auto const corrected_color = sky_correction (color);
				auto const gl_color = to_gl_color (corrected_color);

				glEnable (number);
				glLightfv (number, GL_AMBIENT, gl_color.scaled (0.0f));
				glLightfv (number, GL_DIFFUSE, gl_color.scaled (0.2f));
				glLightfv (number, GL_SPECULAR, gl_color.scaled (0.0f));
				glLightfv (number, GL_POSITION, GLArray { _gl.to_opengl (kSkyHeight), 0.0f, 0.0f, 0.0f });
			});
		}
	}
	else
	{
		// Otherwise let the observer cast the light:
		glLightfv (kSunLight, GL_POSITION, GLArray { 0.0f, 0.0f, 1.0f, 0.0f });

		// Disable sky lights:
		for (auto& sky_light: _sky_lights)
			glDisable (sky_light.gl_number);
	}

	// TODO GL_SPOT_DIRECTION
	// TODO GL_SPOT_EXPONENT
	// TODO GL_SPOT_CUTOFF
}


void
RigidBodyPainter::center_at_followed_object()
{
	glLoadIdentity();
	// Center the world at the followed body:
	_gl.translate (-_camera_position);
	// Rotate about that body:
	apply_camera_rotations();
}


void
RigidBodyPainter::center_at_observer()
{
	glLoadIdentity();
	// Rotate about that body:
	apply_camera_rotations();
}


void
RigidBodyPainter::apply_camera_rotations()
{
	// The camera always rotates about the screen Y axis and can be moved up and down using the screen X axis.
	_gl.rotate_x (_camera_angles[0]); // Pitch
	_gl.rotate_y (_camera_angles[1]); // Yaw
	_gl.rotate_z (_camera_angles[2]); // Roll

	auto const* followed_group = this->followed_group();
	auto const* followed_body = this->followed_body();

	// Match the screen coordinates with the group/body/planet coordinates.
	if ((followed_group || followed_body) && followed_body != _planet_body)
	{
		// Make an exception if we're following the planet body: we don't want to use aircraft coordinates
		// for the planet, because it's unnatural:
		if (followed_body && followed_body == _planet_body)
			apply_screen_to_null_island_rotations();
		else if (_camera_follows_orientation)
		{
			// Now the body will be shown in standard screen coordinates (X = right, Y = top, Z = towards the viewer).
			// But we assume that bodies and groups use aircraft coordinates (X = front, Y = right wing, Z = down).
			// So rotate again to be able to see the aircraft from behind (when camera rotations are all 0°):
			_gl.rotate_z (-90_deg);
			_gl.rotate_y (+90_deg);

			// Rotate body/group to match the screen coordinates:
			if (followed_group)
				if (auto const* rotation_reference_body = followed_group->rotation_reference_body())
					followed_body = rotation_reference_body;

			if (followed_body)
				_gl.rotate (followed_body->placement().body_to_base_rotation());

		}
		else
		{
			// Rotate the world so that down points to the center of _planet_body:
			_gl.rotate_x (+_position_on_earth.lat() - 90_deg);
			_gl.rotate_y (-_position_on_earth.lon());
			apply_screen_to_null_island_rotations();
		}
	}
	// Nothing is followed, so default to planet body if it exists:
	else if (_planet_body)
		apply_screen_to_null_island_rotations();
}


void
RigidBodyPainter::apply_screen_to_null_island_rotations()
{
	// Start with assumption that OpenGL coordinates are equal to ECEF coordinates:
	// X = Null Island, Y = lon/lat 90°/0° and Z = North.

	// Rotate -90° around X-axis to align the Y-axis with the equator
	// (so that Z points towards the prime meridian instead of North):
	_gl.rotate_x (-90_deg);
	// Rotate -90° around Z-axis to shift the X-axis from Null Island towards
	// the prime meridian and align Y with the correct eastward direction:
	_gl.rotate_z (-90_deg);
}


void
RigidBodyPainter::make_z_towards_the_sun()
{
	// Assuming we have identity rotations + camera rotations applied.

	// Rotate sun depending on UTC time:
	auto const greenwich_hour_angle = _sky_dome.sun_position.hour_angle - _position_on_earth.lon();
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
	_gl.rotate_z (+_position_on_earth.lon());
	_gl.rotate_y (-_position_on_earth.lat());
	_gl.rotate_y (+90_deg);
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
		center_at_followed_object();
		setup_natural_light();
		paint_planet();
		paint_air_particles();
		paint (system);
	});
}


void
RigidBodyPainter::paint_planet()
{
	if (!_planet_body)
		return;

	float normalized_altitude = renormalize (_agl_height, Range { 0_km, 15_km }, Range { 0.0f, 1.0f });
	normalized_altitude = std::clamp (normalized_altitude, 0.0f, 1.0f);

	auto const low_fog_color = QColor (0x58, 0x72, 0x92).lighter (200);
	auto const high_fog_color = QColor (0xa5, 0xc9, 0xd3);

	auto const ground_color = QColor (0xaa, 0x55, 0x00).darker (150);
	auto const high_ground_fog_color = high_fog_color;
	auto const low_ground_fog_color = low_fog_color;
	auto const ground_fog_density = renormalize (normalized_altitude, Range { 0.0f, 1.0f }, Range { 0.001f, 0.0015f });

	// Draw stuff like we were located at Lon/Lat 0°/0° looking towards south pole.
	// In other words match ECEF coordinates with standard OpenGL screen coordinates.

	// Offset by planet position in the simulation: // FIXME wrong if the center of the world is focused object?
	_gl.translate (_planet_body->placement().position());

	_gl.save_context ([&] {
		// Sky:
		_gl.save_context ([&] {
			// The sky is centered at the followed body. FIXME It should be centered at the observer
			_gl.rotate_z (+_position_on_earth.lon());
			_gl.rotate_y (-_position_on_earth.lat());
			_gl.rotate_y (90_deg);
			// Normally the outside of the sphere shape is rendered, inside is culled.
			// But here we're inside the sphere, so tell OpenGL that the front faces are the
			// inside faces:
			glFrontFace (GL_CW);
			_gl.draw (_sky_dome.sky_shape);
			glFrontFace (GL_CCW);
		});

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

#if 0
			// For debugging.
			// Draw black balls at the positions of additional source of lights.

			auto black_sun = rigid_body::make_centered_sphere_shape ({
				.radius = 30_m,
				.slices = 10,
				.stacks = 10,
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
#endif

			glFrontFace (GL_CCW);
			glDisable (GL_BLEND);
			glEnable (GL_DEPTH_TEST);
			glEnable (GL_LIGHTING);
		});

		// Ground:
		_gl.save_context ([&] {
			auto const ground_fog_color = hsl_interpolation (normalized_altitude, low_ground_fog_color, high_ground_fog_color);

			rigid_body::ShapeMaterial ground_material;
			ground_material.set_emission_color (ground_color);
			ground_material.set_ambient_color (Qt::black);
			ground_material.set_diffuse_color (Qt::black);
			ground_material.set_specular_color (Qt::black);
			ground_material.set_shininess (0.0f);

			glFogi (GL_FOG_MODE, GL_EXP);
			glFogi (GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
			glFogf (GL_FOG_DENSITY, ground_fog_density);
			glFogf (GL_FOG_START, _gl.to_opengl (0_m));
			glFogf (GL_FOG_END, _gl.to_opengl (kHorizonRadius));
			glFogfv (GL_FOG_COLOR, to_gl_color (ground_fog_color));

			make_z_sky_top_x_south();
			_gl.translate (0_m, 0_m, -_agl_height);

			glEnable (GL_FOG);
			_gl.draw (rigid_body::make_solid_circle (kHorizonRadius, { 0_deg, 360_deg }, 10, ground_material));
			glDisable (GL_FOG);
		});
	});
}


void
RigidBodyPainter::paint_air_particles()
{
	_gl.save_context ([&] {
		// Air 'particles' only appear if we have a planet:
		auto const dust_size = 2_cm;
		auto const grid_size = 5_m;
		auto dust_material = rigid_body::kWhiteMatte;
		dust_material.set_emission_color (Qt::white);
		auto const dust = rigid_body::make_centered_sphere_shape ({ .radius = dust_size, .slices = 3, .stacks = 3, .material = dust_material });
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
	glDisable (GL_FOG);

	_gl.save_context ([&] {
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
			_gl.clear_z_buffer();

			for (auto const& group: system.groups())
				paint_helpers (*group, get_rendering_config (*group), group.get() == focused_group);

			for (auto const& body: system.bodies())
				paint_helpers (*body, get_rendering_config (*body), body.get() == focused_body);

			glDisable (GL_BLEND);
		}
	});
}


template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
	void
	RigidBodyPainter::transform_gl_to (Placement<BaseSpace, Space> const& placement)
	{
		_gl.translate (placement.position());
		_gl.rotate (placement.base_to_body_rotation());
	}


void
RigidBodyPainter::transform_gl_to_center_of_mass (rigid_body::Group const& group)
{
	// Transform so that center-of-mass is at the OpenGL space origin:
	auto const* rotation_reference_body = group.rotation_reference_body();
	auto const rotation = rotation_reference_body
		? rotation_reference_body->placement().body_to_base_rotation()
		: kNoRotation<WorldSpace, BodyCOM>;
	transform_gl_to (Placement<WorldSpace, BodyCOM> (get_center_of_mass (group), rotation) - followed_position());
}


void
RigidBodyPainter::transform_gl_to_center_of_mass (rigid_body::Body const& body)
{
	// Transform so that center-of-mass is at the OpenGL space origin:
	transform_gl_to (body.placement() - followed_position());
}


void
RigidBodyPainter::transform_gl_from_body_center_of_mass_to_origin (rigid_body::Body const& body)
{
	transform_gl_to (body.origin_placement());
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
			auto fp = followed_position();
			auto const& b1 = constraint.body_1();
			auto const& b2 = constraint.body_2();
			auto com1 = b1.placement().position() - fp;
			auto com2 = b2.placement().position() - fp;

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
				paint_basis (_camera_position.z() / 15); // Rotation was taken into account in transform_gl_to_center_of_mass (Group).
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

				paint_basis (_camera_position.z() / 15);
			}
		});
	}
}


void
RigidBodyPainter::paint_center_of_mass()
{
	auto const com_shape = rigid_body::make_center_of_mass_symbol_shape (_camera_position.z() / 150);

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
	auto const origin_shape = rigid_body::make_centered_sphere_shape ({ .radius = _camera_position.z() / 150, .slices = 8, .stacks = 4, .material = origin_material });

	_gl.save_context ([&] {
		setup_feature_light();
		_gl.draw (origin_shape);
		setup_natural_light();
	});
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
	auto const fbp = followed_position();
	auto const com = body.placement().position() - fbp;

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
				auto const at = pl.bound_transform_to_base (forces.center_of_pressure) - fbp;

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
	auto const com = body.placement().position() - followed_position();
	auto const omega = body.velocity_moments<WorldSpace>().angular_velocity();

	draw_arrow (com, omega * angular_velocity_to_length, rigid_body::make_material (Qt::darkMagenta));
}


void
RigidBodyPainter::paint_angular_momentum (rigid_body::Body const& body)
{
	auto constexpr angular_momentum_to_length = 0.001_m / (1_kg * 1_m2 / 1_s) / 1_rad; // TODO unhardcode
	auto const com = body.placement().position() - followed_position();
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

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	glTranslatef (tx, ty, -1.0);
	_gl.set_hfov_perspective (canvas.size(), 60_deg, _gl.to_opengl (1_cm), _gl.to_opengl (10_m));
	glMatrixMode (GL_MODELVIEW);
	glDisable (GL_FOG);

	_gl.save_context ([&] {
		_gl.translate (0_m, 0_m, -1_m);
		apply_camera_rotations();
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
	_gl.draw (rigid_body::make_centered_sphere_shape ({ .radius = 2 * radius, .slices = 8, .stacks = 4 }));
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

	setup_natural_light();
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
RigidBodyPainter::schedule_sky_dome_recalculation()
{
	if (_work_performer)
		_next_sky_dome = _work_performer->submit (&RigidBodyPainter::calculate_sky_dome, this);
	else
		_sky_dome = calculate_sky_dome();
}


// TODO also recalculate on change of followed object and its position > some delta (or in other words: on change of camera position (but not orientation))
SkyDome
RigidBodyPainter::calculate_sky_dome()
{
	return xf::calculate_sky_dome ({
		.atmospheric_scattering = _atmospheric_scattering,
		.observer_position = _position_on_earth,
		.horizon_radius = kHorizonRadius,
		.earth_radius = kEarthMeanRadius,
		.unix_time = _time,
	}, _work_performer);
	// TODO apply sky_correction to vertices' materials
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

