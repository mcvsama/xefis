/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>

// System:
#include <GL/gl.h>
#include <GL/glu.h>

// Qt:
#include <QOpenGLFunctions>
#include <QPainter>

// Neutrino:
#include <neutrino/stdexcept.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/euler_angles.h>
#include <xefis/support/math/tait_bryan_angles.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/simulation/constraints/fixed_constraint.h>
#include <xefis/support/simulation/constraints/hinge_constraint.h>
#include <xefis/support/simulation/constraints/slider_constraint.h>
#include <xefis/support/simulation/devices/wing.h>
#include <xefis/support/simulation/rigid_body/various_shapes.h>
#include <xefis/support/ui/gl_space.h>

// Local:
#include "rigid_body_painter.h"


namespace xf {

RigidBodyPainter::RigidBodyPainter (si::PixelDensity const pixel_density):
	_pixel_density (pixel_density),
	_gl (pixel_density * kDefaultPositionScale)
{ }


void
RigidBodyPainter::paint (rigid_body::System const& system, QOpenGLPaintDevice& canvas)
{
	initializeOpenGLFunctions();

	QPainter painter (&canvas);
	QRectF rect (0, 0, canvas.width(), canvas.height());
	QPointF center = rect.center();

	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);

	painter.translate (center);
	painter.beginNativePainting();
	setup (canvas);
	paint_world (system, canvas);
	paint_ecef_basis (canvas);
	painter.endNativePainting();
}


void
RigidBodyPainter::setup (QOpenGLPaintDevice& canvas)
{
	auto const size = canvas.size();

	_position_on_earth = xf::polar (math::reframe<ECEFSpace, void> (followed_body_position()));

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
	glEnable (GL_LIGHT0);

	glLoadIdentity();
}


void
RigidBodyPainter::setup_camera()
{
	_gl.translate (-_camera_position);
	_gl.rotate (_camera_angles[0], 1.0, 0.0, 0.0);
	_gl.rotate (_camera_angles[1], 0.0, 1.0, 0.0);
	_gl.rotate (_camera_angles[2], 0.0, 0.0, 1.0);

	if (_planet_body)
	{
		_gl.rotate (-_position_on_earth.lon() + 90_deg, 0, 0, 1);
		_gl.rotate (+_position_on_earth.lat(), 0, 1, 0);
	}
}


void
RigidBodyPainter::setup_light()
{
	glLightfv (GL_LIGHT0, GL_AMBIENT, GLArray { 0.25f, 0.25f, 0.25f, 1.0f });
	glLightfv (GL_LIGHT0, GL_DIFFUSE, GLArray { 0.5f, 0.5f, 0.5f, 1.0f });
	glLightfv (GL_LIGHT0, GL_SPECULAR, GLArray { 0.75f, 0.75f, 0.75f, 1.0f });

	_gl.save_matrix ([&] {
		if (_planet_body)
			// For planetary system, try to be sun:
			_gl.translate (kSunDistance, 0_m, 0_m);
		else
			// Otherwise let the observer cast the light:
			_gl.translate (0_m, 0_m, 1_km);

		glLightfv (GL_LIGHT0, GL_POSITION, GLArray { 0.0f, 0.0f, 0.0f, 0.5f });
		// TODO GL_SPOT_DIRECTION
		// TODO GL_SPOT_EXPONENT
		// TODO GL_SPOT_CUTOFF
	});
}


void
RigidBodyPainter::paint_world (rigid_body::System const& system, QOpenGLPaintDevice& canvas)
{
	paint_planet();
	paint_system (system, canvas);
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

	auto const altitude_amsl = abs (followed_body_position() + _camera_position) - kEarthMeanRadius;
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

	_gl.save_matrix ([&] {
		setup_camera();
		setup_light();

		// Draw stuff like we were located at Lon/Lat 0°/0° looking towards south pole.
		// In other words match ECEF coordinates with standard OpenGL screen coordinates.

		// Sky:
		_gl.save_matrix ([&] {
			auto const sky_color = get_intermediate_color (normalized_altitude, sky_low_color, sky_high_color);
			auto const sky_fog_color = get_intermediate_color (normalized_altitude, low_sky_fog_color, high_sky_fog_color);

			auto sky_material = _gl.make_material (Qt::black);
			sky_material.set_shininess (0.0);

			auto const configure_material = [&] (rigid_body::ShapeMaterial& material, si::Angle const latitude)
			{
				// Set dome color (fog simulation) depending on latitude:
				float const norm = std::clamp<float> (renormalize<si::Angle> (latitude, Range { 67.5_deg, 90_deg }, Range { 1.0f, 0.0f }), 0.0f, 1.0f);
				material.set_emission_color (get_intermediate_color (std::pow (norm, 1.0 + 2 * normalized_altitude), sky_color, sky_fog_color));
			};

			auto sky = rigid_body::make_sphere_shape (kEarthMeanRadius + kSkyHeight, 20, 20, { 0_deg, 360_deg }, { 60_deg, 90_deg }, sky_material, configure_material);
			rigid_body::negate_normals (sky);

			_gl.rotate (+_position_on_earth.lon(), 0, 0, 1);
			_gl.rotate (-_position_on_earth.lat(), 0, 1, 0);
			_gl.translate (-kEarthMeanRadius - altitude_amsl, 0_m, 0_m);
			_gl.rotate (+90_deg, 0, 1, 0);

			glFrontFace (GL_CW);
			_gl.draw (sky);
			glFrontFace (GL_CCW);
		});

		// Sun:
		_gl.save_matrix ([&] {
			auto sun_material = _gl.make_material (Qt::black);
			sun_material.set_shininess (0.0);

			auto const configure_material = [&] (rigid_body::ShapeMaterial& material, si::Angle const latitude)
			{
				float const actual_radius = 0.025;
				float const norm = renormalize<si::Angle> (latitude, Range { 0_deg, 90_deg }, Range { 0.0f, 1.0f });
				float const alpha = std::clamp<float> (std::pow (norm + actual_radius, 6.0f), 0.0f, 1.0f);
				material.set_emission_color (QColor (0xff, 0xff, 0xff, 0xff * alpha));
			};

			// Assume it's noon at Lon/Lat 0°/0° right now.
			_gl.translate (kSunDistance, 0_m, 0_km);
			_gl.rotate (+90_deg, 0, 1, 0);
			// Rotate sun shines when camera angle changes:
			_gl.rotate (_camera_angles[0] - 2 * _camera_angles[1], 0, 0, 1);

			auto sun = rigid_body::make_sphere_shape (kSunRadius, 9, 36, { 0_deg, 360_deg }, { 0_deg, 90_deg }, sun_material, configure_material);
			rigid_body::negate_normals (sun);

			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable (GL_ALPHA_TEST);

			glDisable (GL_DEPTH_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_LIGHTING);
			glFrontFace (GL_CW);
			_gl.draw (sun);
			glFrontFace (GL_CCW);
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_BLEND);
			glEnable (GL_LIGHTING);
		});

		// Ground:
		_gl.save_matrix ([&] {
			auto const ground_fog_color = get_intermediate_color (normalized_altitude, low_ground_fog_color, high_ground_fog_color);

			rigid_body::ShapeMaterial ground_material;
			ground_material.set_emission_color (ground_color);
			ground_material.set_ambient_color (Qt::black);
			ground_material.set_diffuse_color (Qt::black);
			ground_material.set_specular_color (Qt::black);
			ground_material.set_shininess (0.0);

			glFogi (GL_FOG_MODE, GL_EXP);
			glFogi (GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
			glFogf (GL_FOG_DENSITY, ground_fog_density);
			glFogf (GL_FOG_START, _gl.to_opengl (0_m));
			glFogf (GL_FOG_END, _gl.to_opengl (kHorizonRadius));
			glFogfv (GL_FOG_COLOR, _gl.to_opengl (ground_fog_color));

			_gl.rotate (+_position_on_earth.lon(), 0, 0, 1);
			_gl.rotate (-_position_on_earth.lat(), 0, 1, 0);
			_gl.translate (-altitude_amsl, 0_m, 0_m);
			_gl.rotate (+90_deg, 0, 1, 0);

			glEnable (GL_FOG);
			_gl.draw (rigid_body::make_solid_circle (kHorizonRadius, 10, ground_material));
			glDisable (GL_FOG);
		});

		// Ground haze that reflects sun a bit:
		_gl.save_matrix ([&] {
			// TODO semi-transparent layer between us and the ground with strong reflection of light
		});
	});
}


void
RigidBodyPainter::paint_system (rigid_body::System const& system, QOpenGLPaintDevice&)
{
	glDisable (GL_FOG);

	_gl.save_matrix ([&] {
		setup_camera();
		setup_light();

		for (auto const& body: system.bodies())
			paint_body (*body);

		if (constraints_visible())
			for (auto const& constraint: system.constraints())
				paint_constraint (*constraint);

		if (forces_visible())
			for (auto const& body: system.bodies())
				paint_forces (*body);

		if (angular_velocities_visible())
			for (auto const& body: system.bodies())
				paint_angular_velocity (*body);

		if (angular_momenta_visible())
			for (auto const& body: system.bodies())
				paint_angular_momentum (*body);
	});
}


void
RigidBodyPainter::paint_body (rigid_body::Body const& body)
{
	_gl.save_matrix ([&] {
		auto const translation = body.location().position() - followed_body_position();

		_gl.translate (translation);
		_gl.rotate (body.location().base_to_body_rotation());

		if (auto const& shape = body.shape())
			_gl.draw (*shape);
		else
		{
			auto const edge = _mass_scale * 1_kg * std::pow (body.mass_moments<rigid_body::BodySpace>().mass() / 1_kg, 1.0 / 3);
			_gl.draw (rigid_body::make_cube_shape (edge));
		}
	});
}


void
RigidBodyPainter::paint_constraint (rigid_body::Constraint const& constraint)
{
	if (constraint.enabled() && !constraint.broken())
	{
		_gl.save_matrix ([&] {
			auto fcorr = followed_body_position();
			auto const& b1 = constraint.body_1();
			auto const& b2 = constraint.body_2();
			auto x1 = b1.location().position() - fcorr;
			auto x2 = b2.location().position() - fcorr;

			auto const rod_from_to = [this] (si::Length const radius, auto const& from, auto const& to, bool front_back_faces, rigid_body::ShapeMaterial const& material)
			{
				_gl.save_matrix ([&] {
					auto const diff = to - from;
					_gl.translate (from);

					auto const alpha_beta = alpha_beta_from_x_to (diff);
					_gl.rotate (alpha_beta[0], 0, 0, 1);
					_gl.rotate (alpha_beta[1], 0, 1, 0);
					_gl.rotate (90_deg, 0, 1, 0);

					_gl.draw (rigid_body::make_cylinder_shape (1_m * abs (diff / 1_m), radius, 16, front_back_faces, material));
				});
			};

			if (auto const* hinge = dynamic_cast<rigid_body::HingeConstraint const*> (&constraint))
			{
				auto const r1 = b1.location().unbound_transform_to_base (hinge->hinge_precalculation().body_1_anchor());
				auto const r2 = b2.location().unbound_transform_to_base (hinge->hinge_precalculation().body_2_anchor());
				auto const t1 = x1 + r1;
				auto const t2 = x2 + r2;
				auto const material = _gl.make_material (QColor (0xff, 0x99, 0x00));

				rod_from_to (kDefaultConstraintDiameter, x1, t1, false, material);
				rod_from_to (kDefaultConstraintDiameter, x2, t2, false, material);

				// Hinge:
				if (auto const& data = hinge->hinge_precalculation().data())
				{
					auto const h = 1.5 * kDefaultConstraintDiameter * normalized (data->a1);
					rod_from_to (kDefaultHingeDiameter, t1 - h, t1 + h, true, material);
				}
			}
			else if (dynamic_cast<rigid_body::FixedConstraint const*> (&constraint))
			{
				auto const material = _gl.make_material (QColor (0xff, 0x00, 0x99));
				rod_from_to (kDefaultConstraintDiameter, x1, x2, false, material);
			}
		});
	}
}


void
RigidBodyPainter::paint_forces (rigid_body::Body const& body)
{
	bool const show_gravity = false;
	bool const show_aerodynamic_forces = true;

	auto const gravity_color = Qt::magenta;
	auto const lift_color = Qt::green;
	auto const drag_color = Qt::red;
	auto const torque_color = Qt::blue;
	auto const external_force_color = Qt::green;
	auto const external_torque_color = Qt::cyan;

	auto const force_to_length = 0.1_m / 1_N; // TODO unhardcode
	auto const torque_to_length = force_to_length / 1_m; // TODO unhardcode

	auto const& cache = body.frame_cache();
	auto const gfm = cache.gravitational_force_moments;
	auto const efm = cache.external_force_moments;
	auto const fbp = followed_body_position();
	auto const com = body.location().position() - fbp;

	if (show_gravity)
		draw_arrow (com, gfm.force() * force_to_length, _gl.make_material (gravity_color));

	if (auto const* wing = dynamic_cast<sim::Wing const*> (&body))
	{
		auto const& loc = wing->location();
		auto const at = loc.bound_transform_to_base (wing->center_of_pressure()) - fbp;

		if (show_aerodynamic_forces)
		{
			draw_arrow (at, loc.unbound_transform_to_base (wing->lift_force()) * force_to_length, _gl.make_material (lift_color));
			draw_arrow (at, loc.unbound_transform_to_base (wing->drag_force()) * force_to_length, _gl.make_material (drag_color));
			draw_arrow (at, loc.unbound_transform_to_base (wing->pitching_moment()) * torque_to_length, _gl.make_material (torque_color));
		}
	}

	if (!show_aerodynamic_forces)
		draw_arrow (com, efm.force() * force_to_length, _gl.make_material (external_force_color));

	draw_arrow (com, efm.torque() * torque_to_length, _gl.make_material (external_torque_color));
}


void
RigidBodyPainter::paint_angular_velocity (rigid_body::Body const& body)
{
	auto const angular_velocity_to_length = 0.1_m / 1_radps; // TODO unhardcode
	// TODO auto const& cache = body.frame_cache();
	auto const com = body.location().position() - followed_body_position();
	auto const omega = body.velocity_moments<rigid_body::WorldSpace>().angular_velocity();

	draw_arrow (com, omega * angular_velocity_to_length, _gl.make_material (Qt::darkMagenta));
}


void
RigidBodyPainter::paint_angular_momentum (rigid_body::Body const& body)
{
	auto const angular_momentum_to_length = 0.001_m / (1_kg * 1_m2 / 1_s) / 1_rad; // TODO unhardcode
	//TODO auto const& cache = body.frame_cache();
	auto const com = body.location().position() - followed_body_position();
	auto const I = body.mass_moments<rigid_body::BodySpace>().moment_of_inertia();
	auto const L = I * body.velocity_moments<rigid_body::BodySpace>().angular_velocity();
	auto const L_world = body.location().unbound_transform_to_base (L);

	draw_arrow (com, L_world * angular_momentum_to_length, _gl.make_material (Qt::darkBlue));
}


void
RigidBodyPainter::draw_arrow (SpaceLength<rigid_body::WorldSpace> const& origin, SpaceLength<rigid_body::WorldSpace> const& vector, rigid_body::ShapeMaterial const& material)
{
	_gl.save_matrix ([&] {
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
			_gl.draw (rigid_body::make_cylinder_shape (length, radius, kNumFaces, true, material));
			_gl.translate (0_m, 0_m, length);
			_gl.draw (rigid_body::make_cone_shape (cone_length, cone_radius, kNumFaces, true, material));
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

	float const scale = 0.4;
	si::Length const radius = 4_mm * scale;
	si::Length const length = 20_cm * scale;
	si::Length const cone_radius = 15_mm * scale;
	si::Length const cone_length = 40_mm * scale;

	auto const blue = _gl.make_material (QColor (0x11, 0x11, 0xff));
	auto const red = _gl.make_material (Qt::red);
	auto const green = _gl.make_material (Qt::green);

	auto const draw = [&] {
		_gl.save_matrix ([&] {
			glEnable (GL_LIGHT1);
			glLightfv (GL_LIGHT1, GL_POSITION, GLArray { 0.0f, 0.0f, 0.0f, 0.5f });
			glLightfv (GL_LIGHT1, GL_AMBIENT, GLArray { 0.25f, 0.25f, 0.25f, 1.0f });
			glLightfv (GL_LIGHT1, GL_DIFFUSE, GLArray { 0.5f, 0.5f, 0.5f, 1.0f });
			glLightfv (GL_LIGHT1, GL_SPECULAR, GLArray { 0.9f, 0.9f, 0.9f, 1.0f });

			_gl.translate (0_m, 0_m, -1_m);
			_gl.rotate (_camera_angles[0], 1.0, 0.0, 0.0);
			_gl.rotate (_camera_angles[1], 0.0, 1.0, 0.0);
			_gl.rotate (_camera_angles[2], 0.0, 0.0, 1.0);

			if (_planet_body)
			{
				_gl.rotate (-_position_on_earth.lon() + 90_deg, 0, 0, 1);
				_gl.rotate (+_position_on_earth.lat(), 0, 1, 0);
			}

			auto const kNumFaces = 12;

			// Root ball:
			_gl.draw (rigid_body::make_sphere_shape (2 * radius, 8, 8));
			// X axis:
			_gl.save_matrix ([&] {
				_gl.rotate (+90_deg, 0.0, 1.0, 0.0);
				_gl.draw (rigid_body::make_cylinder_shape (length, radius, kNumFaces, false, blue));
				_gl.translate (0_m, 0_m, length);
				_gl.draw (rigid_body::make_cone_shape (cone_length, cone_radius, kNumFaces, true, blue));
			});
			// Y axis:
			_gl.save_matrix ([&] {
				_gl.rotate (-90_deg, 1.0, 0.0, 0.0);
				_gl.draw (rigid_body::make_cylinder_shape (length, radius, kNumFaces, false, red));
				_gl.translate (0_m, 0_m, length);
				_gl.draw (rigid_body::make_cone_shape (cone_length, cone_radius, kNumFaces, true, red));
			});
			// Z axis:
			_gl.save_matrix ([&] {
				_gl.draw (rigid_body::make_cylinder_shape (length, radius, kNumFaces, false, green));
				_gl.translate (0_m, 0_m, length);
				_gl.draw (rigid_body::make_cone_shape (cone_length, cone_radius, kNumFaces, true, green));
			});

			glDisable (GL_LIGHT1);
		});
	};

	// Draw once to set z-buffer to farthest value:
	glDepthRange (1.0, 1.0);
	glDepthFunc (GL_ALWAYS);
	glDisable (GL_LIGHTING);
	draw();
	// Draw again, normally. This ensures that basis is always drawn regardless of any other object positions.
	glDepthRangef (0.0, 1.0);
	glDepthFunc (GL_LEQUAL);
	glEnable (GL_LIGHTING);
	draw();
}


SpaceLength<rigid_body::WorldSpace>
RigidBodyPainter::followed_body_position() const
{
	if (_followed_body)
		return _followed_body->location().position();
	else
		return { 0_m, 0_m, 0_m };
}

} // namespace xf

