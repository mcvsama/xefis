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

// Qt:
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


RigidBodyPainter::RigidBodyPainter (si::PixelDensity const pixel_density):
	_pixel_density (pixel_density),
	_gl (pixel_density * kDefaultPositionScale)
{ }


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
	auto const size = canvas.size();

	_position_on_earth = xf::polar (math::coordinate_system_cast<ECEFSpace, void> (followed_position()));

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	glTranslatef (0.0, 0.0, -1.0);
	_gl.set_hfov_perspective (size, 60_deg, _gl.to_opengl (1_m), _gl.to_opengl (100_km));

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
	glEnable (kSunLight);

	glLoadIdentity();
}


void
RigidBodyPainter::setup_feature_light()
{
	glDisable (kSunLight);
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
	glDisable (kFeatureLight);
	glEnable (kSunLight);
}


void
RigidBodyPainter::setup_camera()
{
	_gl.translate (-_camera_position);
	apply_camera_rotations();
}


void
RigidBodyPainter::apply_camera_rotations()
{
	// Remember that in OpenGL transforms are applied in the reverse order.

	_gl.rotate (_camera_angles[0], 1, 0, 0); // Pitch
	_gl.rotate (_camera_angles[1], 0, 1, 0); // Yaw
	_gl.rotate (_camera_angles[2], 0, 0, 1); // Roll

	auto const* followed_group = this->followed_group();
	auto const* followed_body = this->followed_body();

	// Match the screen coordinates with the group/body/planet coordinates:
	if ((followed_group || followed_body) && _following_orientation)
	{
		// The body/group is assumed to be in aircraft coordinates (X = front, Y = right, Z = down).
		// The screen is in standard math/screen coordinates (X = right, Y = top, Z = towards the viewer).
		// We have to rotate about axes defined by the screen to get to see the aircraft from behind.
		_gl.rotate (-90_deg, 0, 0, 1); // About Z
		_gl.rotate (+90_deg, 0, 1, 0); // About Y
	}
	else if (_planet_body)
	{
		// Rotate so that down on the screen is towards the center of the planet.
		_gl.rotate (-_position_on_earth.lon() + 90_deg, 0, 0, 1); // About Z
		_gl.rotate (+_position_on_earth.lat(), 0, 1, 0); // About Y
	}

	if (_following_orientation)
	{
		if (followed_group)
			if (auto const* rotation_reference_body = followed_group->rotation_reference_body())
				followed_body = rotation_reference_body;

		if (followed_body)
			_gl.rotate (followed_body->placement().body_to_base_rotation());
	}
}


void
RigidBodyPainter::setup_light()
{
	glLightfv (kSunLight, GL_AMBIENT, GLArray { 0.25f, 0.25f, 0.25f, 1.0f });
	glLightfv (kSunLight, GL_DIFFUSE, GLArray { 0.5f, 0.5f, 0.5f, 1.0f });
	glLightfv (kSunLight, GL_SPECULAR, GLArray { 0.75f, 0.75f, 0.75f, 1.0f });

	if (_planet_body)
		// For planetary system, try to be sun:
		glLightfv (kSunLight, GL_POSITION, GLArray { _gl.to_opengl (kSunDistance), 0.0f, 0.0f, 0.0f });
	else
		// Otherwise let the observer cast the light:
		glLightfv (kSunLight, GL_POSITION, GLArray { 0.0f, 0.0f, 1.0f, 0.0f });

	// TODO GL_SPOT_DIRECTION
	// TODO GL_SPOT_EXPONENT
	// TODO GL_SPOT_CUTOFF
}


void
RigidBodyPainter::paint_world (rigid_body::System const& system)
{
	_gl.save_context ([&] {
		setup_camera();
		setup_light();

		paint_planet();
		paint_air_particles();
		paint (system);
	});
}


void
RigidBodyPainter::paint_planet()
{
	// TODO Earth's angle 27° depending on time of year
	// TODO Sun's position depending on time of day

	if (!_planet_body)
		return;

	auto const get_intermediate_color = [](float x, QColor const& c1, QColor const& c2)
	{
		qreal h1, s1, l1;
		qreal h2, s2, l2;

		c1.convertTo (QColor::Hsl).getHslF (&h1, &s1, &l1);
		c2.convertTo (QColor::Hsl).getHslF (&h2, &s2, &l2);

		auto const y = 1.0 - x;
		auto const h3 = y * h1 + x * h2;
		auto const s3 = y * s1 + x * s2;
		auto const l3 = y * l1 + x * l2;

		return QColor::fromHslF (h3, s3, l3).convertTo (QColor::Rgb);
	};

	auto const altitude_amsl = abs (followed_position() + _camera_position) - kEarthMeanRadius;
	float normalized_altitude = renormalize (altitude_amsl, Range { 0_km, 15_km }, Range { 0.0f, 1.0f });
	normalized_altitude = std::clamp (normalized_altitude, 0.0f, 1.0f);

	auto const low_fog_color = QColor (0x58, 0x72, 0x92).lighter (200);
	auto const high_fog_color = QColor (0xa5, 0xc9, 0xd3);

	auto const sky_high_color = QColor (0x00, 0x03, 0x20);
	auto const sky_low_color = QColor (0x4d, 0x6c, 0x92);
	auto const high_sky_fog_color = high_fog_color;
	auto const low_sky_fog_color = low_fog_color;

	auto const ground_color = QColor (0xaa, 0x55, 0x00).darker (150);
	auto const high_ground_fog_color = high_fog_color;
	auto const low_ground_fog_color = low_fog_color;
	auto const ground_fog_density = renormalize (normalized_altitude, Range { 0.0f, 1.0f }, Range { 0.001f, 0.0015f });

	// Offset by planet position in the simulation:
	_gl.translate (_planet_body->placement().position());

	// Draw stuff like we were located at Lon/Lat 0°/0° looking towards south pole.
	// In other words match ECEF coordinates with standard OpenGL screen coordinates.

	// Sky:
	_gl.save_context ([&] {
		auto const sky_color = get_intermediate_color (normalized_altitude, sky_low_color, sky_high_color);
		auto const sky_fog_color = get_intermediate_color (normalized_altitude, low_sky_fog_color, high_sky_fog_color);

		auto sky_material = rigid_body::kBlackMatte;

		auto const configure_material = [&] (rigid_body::ShapeMaterial& material, si::Angle const latitude)
		{
			// Set dome color (fog simulation) depending on latitude:
			float const norm = std::clamp<float> (renormalize<si::Angle> (latitude, Range { 67.5_deg, 90_deg }, Range { 1.0f, 0.0f }), 0.0f, 1.0f);
			material.set_emission_color (get_intermediate_color (std::pow (norm, 1.0 + 2 * normalized_altitude), sky_color, sky_fog_color));
		};

		auto sky = rigid_body::make_centered_sphere_shape ({
			.radius = kEarthMeanRadius + kSkyHeight,
			.slices = 20,
			.stacks = 20,
			.v_range = { 60_deg, 90_deg },
			.material = sky_material,
			.setup_material = configure_material,
		});
		rigid_body::negate_normals (sky);

		_gl.rotate (+_position_on_earth.lon(), 0, 0, 1);
		_gl.rotate (-_position_on_earth.lat(), 0, 1, 0);
		_gl.translate (-kEarthMeanRadius - altitude_amsl, 0_m, 0_m);
		_gl.rotate (+90_deg, 0, 1, 0);

		// Normally the outside of the sphere shape is rendered, inside is culled.
		// But here we're inside the sphere, so tell OpenGL that the front faces are the
		// inside faces:
		glFrontFace (GL_CW);
		_gl.draw (sky);
		glFrontFace (GL_CCW);
	});

	// Sun:
	_gl.save_context ([&] {
		auto sun_material = rigid_body::kBlackMatte;

		auto const configure_material = [&] (rigid_body::ShapeMaterial& material, si::Angle const latitude)
		{
			float const actual_radius = 0.025;
			float const norm = renormalize<si::Angle> (latitude, Range { 0_deg, 90_deg }, Range { 0.0f, 1.0f });
			float const alpha = std::clamp<float> (std::pow (norm + actual_radius, 6.0f), 0.0f, 1.0f);
			material.gl_emission_color = { 1.0f, 1.0f, 1.0f, alpha };
		};

		// Assume it's noon at Lon/Lat 0°/0° right now.
		_gl.translate (kSunDistance, 0_m, 0_km);
		_gl.rotate (+90_deg, 0, 1, 0);
		// Rotate sun shines when camera angle changes:
		_gl.rotate (_camera_angles[0] - 2 * _camera_angles[1], 0, 0, 1);

		auto sun = rigid_body::make_centered_sphere_shape ({
			.radius = kSunRadius,
			.slices = 9,
			.stacks = 36,
			.v_range = { 0_deg, 90_deg },
			.material = sun_material,
			.setup_material = configure_material,
		});
		rigid_body::negate_normals (sun);

		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable (GL_ALPHA_TEST);

		glDisable (GL_DEPTH_TEST);
		glEnable (GL_BLEND);
		glDisable (GL_LIGHTING);
		// Same remark as for sky rendering about what's considered the front face here:
		glFrontFace (GL_CW);
		_gl.draw (sun);
		glFrontFace (GL_CCW);
		glEnable (GL_DEPTH_TEST);
		glDisable (GL_BLEND);
		glEnable (GL_LIGHTING);
	});

	// Ground:
	_gl.save_context ([&] {
		auto const ground_fog_color = get_intermediate_color (normalized_altitude, low_ground_fog_color, high_ground_fog_color);

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

		_gl.rotate (+_position_on_earth.lon(), 0, 0, 1);
		_gl.rotate (-_position_on_earth.lat(), 0, 1, 0);
		_gl.translate (-altitude_amsl, 0_m, 0_m);
		_gl.rotate (+90_deg, 0, 1, 0);

		glEnable (GL_FOG);
		_gl.draw (rigid_body::make_solid_circle (kHorizonRadius, { 0_deg, 360_deg }, 10, ground_material));
		glDisable (GL_FOG);
	});

	// Ground haze that reflects sun a bit:
	_gl.save_context ([&] {
		// TODO semi-transparent layer between us and the ground with strong reflection of light
	});
}


void
RigidBodyPainter::paint_air_particles()
{
	_gl.save_context ([&] {
		// Air 'particles' only appear if we have a planet:
		auto const ball_size = 2_cm;
		auto const grid_size = 5_m;
		auto ball_material = rigid_body::kWhiteMatte;
		ball_material.set_emission_color (Qt::white);
		auto const ball = rigid_body::make_centered_sphere_shape ({ .radius = ball_size, .slices = 3, .stacks = 3, .material = ball_material });
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

		// Paint a grid of little balls around the camera position:
		for (auto x = rounded_to_grid.x() - range; x <= rounded_to_grid.x() + range; x += grid_size)
		{
			for (auto y = rounded_to_grid.y() - range; y <= rounded_to_grid.y() + range; y += grid_size)
			{
				for (auto z = rounded_to_grid.z() - range; z <= rounded_to_grid.z() + range; z += grid_size)
				{
					// (x + y + z) makes adjacent points to be wiggled the same amount, so multiply other axes by 2 and 3:
					_air_particles_prng.seed ((x + 2 * y + 3 * z).in<si::Meter>());
					_gl.save_context ([&] {
						// OpenGL's center of the view [0, 0, 0] is at body_pos, hence subtracting body_pos from
						// absolute values (in WorldOrigin space) x, y, z here:
						auto const wiggled_x = x - body_pos.x() + prng_factor * _air_particles_prng() - half_grid_size;
						auto const wiggled_y = y - body_pos.y() + prng_factor * _air_particles_prng() - half_grid_size;
						auto const wiggled_z = z - body_pos.z() + prng_factor * _air_particles_prng() - half_grid_size;
						_gl.translate (wiggled_x, wiggled_y, wiggled_z);
						_gl.draw (ball);
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
				paint_constraint (*constraint);

		if (gravity_visible() || aerodynamic_forces_visible() || external_forces_visible())
			for (auto const& body: system.bodies())
				paint_forces (*body);

		if (angular_velocities_visible())
			for (auto const& body: system.bodies())
				paint_angular_velocity (*body);

		if (angular_momenta_visible())
			for (auto const& body: system.bodies())
				paint_angular_momentum (*body);

		auto const* focused_group = this->focused_group();
		auto const* focused_body = this->focused_body();

		{
			// We'll now paint features that are always visible, so clear the Z buffer
			// in OpenGL and do the painting with blending enabled.

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
				else if (_hovered_body == &body)
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
	auto const origin_shape = rigid_body::make_centered_sphere_shape ({ .radius = _camera_position.z() / 150, .slices = 8, .stacks = 8, .material = origin_material });

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
RigidBodyPainter::paint_constraint (rigid_body::Constraint const& constraint)
{
	if (constraint.enabled() && !constraint.broken())
	{
		_gl.save_context ([&] {
			auto fcorr = followed_position();
			auto const& b1 = constraint.body_1();
			auto const& b2 = constraint.body_2();
			auto com1 = b1.placement().position() - fcorr;
			auto com2 = b2.placement().position() - fcorr;

			auto const rod_from_to = [this] (si::Length const radius, auto const& from, auto const& to, bool front_back_faces, rigid_body::ShapeMaterial const& material)
			{
				_gl.save_context ([&] {
					auto const diff = to - from;
					_gl.translate (from);

					auto const alpha_beta = alpha_beta_from_x_to (diff);
					_gl.rotate (alpha_beta[0], 0, 0, 1);
					_gl.rotate (alpha_beta[1], 0, 1, 0);
					_gl.rotate (90_deg, 0, 1, 0);

					auto const shape = rigid_body::make_cylinder_shape ({
						.length = 1_m * abs (diff / 1_m),
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
				auto const material = rigid_body::make_material (QColor (0xff, 0x99, 0x00));

				// Lines from COM to hinge center:
				rod_from_to (kDefaultConstraintDiameter, com1, hinge_center, false, material);
				rod_from_to (kDefaultConstraintDiameter, com2, hinge_center, false, material);
				// Hinge:
				rod_from_to (kDefaultHingeDiameter, hinge_start_1, hinge_end_1, true, material);
			}
			else if (dynamic_cast<rigid_body::FixedConstraint const*> (&constraint))
			{
				auto const material = rigid_body::make_material (QColor (0xff, 0x00, 0x99));
				rod_from_to (kDefaultConstraintDiameter, com1, com2, false, material);
			}
		});
	}
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
			float const scale = 2.0;
			auto const kNumFaces = 12;
			auto const cone_radius = 20_mm * scale;
			auto const cone_length = 50_mm * scale;
			auto const radius = 5_mm * scale;
			auto const alpha_beta = alpha_beta_from_x_to (vector);

			_gl.translate (origin);
			_gl.rotate (alpha_beta[0], 0, 0, 1);
			_gl.rotate (alpha_beta[1], 0, 1, 0);
			_gl.rotate (90_deg, 0, 1, 0);
			_gl.draw (rigid_body::make_cylinder_shape ({ .length = length, .radius = radius, .num_faces = kNumFaces, .with_bottom = true, .with_top = true, .material = material }));
			_gl.translate (0_m, 0_m, length);
			_gl.draw (rigid_body::make_cone_shape ({ .length = cone_length, .radius = cone_radius, .num_faces = kNumFaces, .with_bottom = true, .material = material }));
		}
	});
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
	_gl.draw (rigid_body::make_centered_sphere_shape ({ .radius = 2 * radius, .slices = 8, .stacks = 8 }));
	// X axis:
	_gl.save_context ([&] {
		_gl.rotate (+90_deg, 0.0, 1.0, 0.0);
		_gl.draw (rigid_body::make_cylinder_shape ({ .length = length, .radius = radius, .num_faces = kNumFaces, .material = red }));
		_gl.translate (0_m, 0_m, length);
		_gl.draw (rigid_body::make_cone_shape ({ .length = cone_length, .radius = cone_radius, .num_faces = kNumFaces, .with_bottom = true, .material = red }));
	});
	// Y axis:
	_gl.save_context ([&] {
		_gl.rotate (-90_deg, 1.0, 0.0, 0.0);
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

} // namespace xf

