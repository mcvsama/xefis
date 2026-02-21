/* vim:ts=4
 *
 * Copyleft 2018  Michał Gawron
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
#include <xefis/support/color/hsl.h>
#include <xefis/support/color/spaces.h>
#include <xefis/support/math/rotations.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/shapes/shape_utils.h>
#include <xefis/support/shapes/various_materials.h>
#include <xefis/support/shapes/various_shapes.h>
#include <xefis/support/simulation/constraints/fixed_constraint.h>
#include <xefis/support/simulation/constraints/hinge_constraint.h>
#include <xefis/support/simulation/constraints/slider_constraint.h>
#include <xefis/support/simulation/devices/wing.h>
#include <xefis/support/ui/gl_space.h>
#include <xefis/support/ui/paint_helper.h>
#include <xefis/support/universe/earth/utility.h>
#include <xefis/support/universe/julian_calendar.h>
#include <xefis/support/universe/moon_position.h>

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
#include <cmath>
#include <cstddef>
#include <future>
#include <limits>
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

namespace {

/**
 * Compute a conservative body-space bounding sphere radius around the center of mass.
 *
 * This is used as a cheap broad-phase test before the expensive exact
 * ray-vs-mesh intersection in body_under_cursor(). If no vertex exists,
 * the radius stays 0_m and we skip that body in picking.
 */
[[nodiscard]]
si::Length
body_picking_radius (rigid_body::Body const& body, Shape const& shape)
{
	auto radius = 0_m;

	shape.for_each_vertex ([&] (ShapeVertex const& vertex) {
		auto const vertex_in_com = body.origin_placement().rotate_translate_to_base (vertex.position());
		radius = std::max (radius, abs (vertex_in_com));
	});

	return radius;
}

} // namespace


nu::Synchronized<std::shared_future<RigidBodyPainter::PlanetTextureImages>>		RigidBodyPainter::_planet_texture_images;
nu::Synchronized<std::shared_future<RigidBodyPainter::UniverseTextureImages>>	RigidBodyPainter::_universe_texture_images;

std::array<RigidBodyPainter::SkyLight, 5> const RigidBodyPainter::Planet::sky_lights = {{
	{
		.gl_number	= kGLSkyLight0,
		.position	= { 0_deg, 0_deg },
	},
	{
		.gl_number	= kGLSkyLight1,
		.position	= { 90_deg, 5_deg },
	},
	{
		.gl_number	= kGLSkyLight2,
		.position	= { 180_deg, 5_deg },
	},
	{
		.gl_number	= kGLSkyLight3,
		.position	= { 270_deg, 5_deg },
	},
	{
		.gl_number	= kGLSkyLight4,
		.position	= { 0_deg, 90_deg },
	},
}};


RigidBodyPainter::RigidBodyPainter (si::PixelDensity const pixel_density, nu::WorkPerformer* work_performer):
	_pixel_density (pixel_density),
	// Work performer needs at least two threads: compute_sky_dome_shape() will be placed as a task, but it will
	// also create its own subtasks. If it would only have 1 thread, the computation subtasks would
	// infinitely wait for the first task to end (deadlock).
	_work_performer (work_performer, std::in_place, 2, nu::Logger()),
	_gl (pixel_density * kDefaultPositionScale)
{
	if (work_performer && work_performer->threads_number() < 2)
		throw nu::InvalidArgument ("provided WorkPerformer must have at least 2 threads");

	use_work_performer (work_performer);
	compute_camera_transform();
}


bool
RigidBodyPainter::ready() const
{
	return (!_planet || nu::valid_and_ready (*_planet_texture_images.lock()))
		&& (!_universe || nu::valid_and_ready (*_universe_texture_images.lock()));
}


void
RigidBodyPainter::set_time (si::Time const new_time)
{
	_time = new_time;

	if (abs (_time - _prev_saved_time) > 5_s)
	{
		if (_planet)
			_planet->sky_dome_shape.reset();

		if (_universe)
			check_sky_box();

		_prev_saved_time = _time;
	}
}


void
RigidBodyPainter::set_followed_to_none()
{
	_followed = std::monostate();
	compute_followed_position();
	compute_camera_transform();
}


void
RigidBodyPainter::set_camera_mode (CameraMode const mode)
{
	_camera_mode = mode;
	compute_camera_transform();
}


void
RigidBodyPainter::set_camera_position (si::LonLatRadius<> const position)
{
	_requested_camera_polar_position = position;
	compute_camera_transform();
}


void
RigidBodyPainter::set_user_camera_translation (SpaceLength<WorldSpace> const& translation)
{
	_user_camera_translation = translation;
	compute_camera_transform();
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
		compute_camera_transform();
	}
}


si::Length
RigidBodyPainter::camera_distance_to_followed() const
{
	return (_followed_position - _camera.position()).norm();
}


void
RigidBodyPainter::set_planet (rigid_body::Body const* planet_body)
{
	if (planet_body)
	{
		_planet.emplace();
		_planet->body = planet_body;
		check_planet_texture_images();
	}
	else
	{
		_planet.reset();
		_moon_shape.reset();
	}

	compute_camera_transform();
}


void
RigidBodyPainter::set_sun_enabled (bool enabled)
{
	if (enabled)
		_sun.emplace();
	else
		_sun.reset();

	if (_planet)
		_planet->sky_dome_shape.reset();
}


void
RigidBodyPainter::set_universe_enabled (bool enabled)
{
	if (enabled)
	{
		_universe.emplace();
		check_universe_texture_images();
		check_sky_box();
	}
	else
		_universe.reset();
}


rigid_body::Body const*
RigidBodyPainter::body_under_cursor (rigid_body::System const& system, QPoint const& cursor_position, QSize const& viewport_size)
{
	if (viewport_size.width() <= 0 || viewport_size.height() <= 0)
		return nullptr;

	// Build a world-space picking ray from cursor pixel coordinates.
	// Pixel center (+0.5) is used to avoid edge bias, then converted to NDC and
	// projected through the current perspective (FOV + aspect ratio).
	auto const width = static_cast<double> (viewport_size.width());
	auto const height = static_cast<double> (viewport_size.height());
	auto const ndc_x = 2.0 * (cursor_position.x() + 0.5) / width - 1.0;
	auto const ndc_y = 1.0 - 2.0 * (cursor_position.y() + 0.5) / height;
	auto const tan_half_fov = tan (0.5 * _fov);
	auto const aspect = width / height;
	auto const ray_direction_camera = SpaceVector<double, WorldSpace> {
		ndc_x * tan_half_fov * aspect,
		ndc_y * tan_half_fov,
		-1.0,
	}.normalized();
	auto const ray_direction_world = (_camera.base_rotation() * ray_direction_camera).normalized();
	auto const ray_origin_world = _camera.position();
	auto nearest_distance = std::numeric_limits<double>::infinity() * 1_m;
	rigid_body::Body const* nearest_body = nullptr;

	for (auto const& body_ptr: system.bodies())
	{
		auto const& body = *body_ptr;

		if (get_rendering_config (body).body_visible)
		{
			auto const& body_shape = shape_for (body);

			if (auto const radius = body_picking_radius (body, body_shape);
				radius > 0_m)
			{
				// radius must be > 0_m, otherwise sphere intersection is degenerate
				// (point sphere / empty geometry), so there is nothing meaningful to pick.
				// This quick sphere hit test rejects bodies definitely not under the cursor,
				// avoiding costly per-shape intersection for most bodies.
				if (auto const bounding_sphere_distance = ray_sphere_intersection (ray_origin_world, ray_direction_world, body.placement().position(), radius);
					bounding_sphere_distance && *bounding_sphere_distance < nearest_distance)
				{
					// bounding_sphere_distance gives the first hit distance along the ray.
					// Keeping only distances nearer than current best lets us prune farther
					// candidates before transforming rays and running exact mesh picking.
					auto const ray_origin_body_com = body.placement().rotate_translate_to_body (ray_origin_world);
					auto const ray_origin_body_origin = body.origin_placement().rotate_translate_to_body (ray_origin_body_com);
					auto const ray_direction_body_origin = body.origin_placement().rotate_to_body (body.placement().rotate_to_body (ray_direction_world)).normalized();

					// Narrow-phase: exact intersection with the body shape in body-origin space.
					if (auto const shape_distance = ray_shape_intersection (body_shape, ray_origin_body_origin, ray_direction_body_origin))
					{
						if (*shape_distance < nearest_distance)
						{
							nearest_distance = *shape_distance;
							nearest_body = &body;
						}
					}
				}
			}
		}
	}

	return nearest_body;
}


void
RigidBodyPainter::paint (rigid_body::System const& system, QOpenGLPaintDevice& canvas)
{
	_group_centers_of_mass_cache.clear();
	initializeOpenGLFunctions();

	// Paint black background, reset z-buffer and stencil buffer:
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	auto const ph = PaintHelper (canvas);
	auto painter = QPainter (&canvas);
	ph.setup_painter (painter);

	QRectF rect (0, 0, canvas.width(), canvas.height());
	QPointF center = rect.center();

	painter.translate (center);
	painter.beginNativePainting();
	precompute();
	setup_camera (canvas);
	setup_lights();
	check_textures();
	check_sky_dome_and_ground_shape();
	paint_world (system);
	paint_ecef_basis (canvas);
	painter.endNativePainting();
}


void
RigidBodyPainter::drop_resources()
{
	_planet_texture_images.store ({});
	_universe_texture_images.store ({});
}


void
RigidBodyPainter::precompute()
{
	if (_planet)
	{
		auto const followed_body_normalized_height = nu::renormalize (_followed_polar_position.radius(), nu::Range { kEarthMeanRadius, kAtmosphereRadius }, nu::Range { 0.0f, 1.0f });
		_planet->horizon_angle = compute_horizon_angle (kEarthMeanRadius, _camera_polar_position.radius());
		_planet->followed_body_normalized_amsl_height = std::clamp<float> (followed_body_normalized_height, 0.0f, 1.0f);
		_planet->camera_normalized_amsl_height = nu::renormalize (_camera_polar_position.radius(), nu::Range { kEarthMeanRadius, kAtmosphereRadius }, nu::Range { 0.0f, 1.0f });
		_planet->camera_clamped_normalized_amsl_height = std::clamp<float> (_planet->camera_normalized_amsl_height, 0.0f, 1.0f);
	}

	compute_followed_position();

	if (_sun)
	{
		_sun->position = compute_sun_position (_camera_polar_position, _time);
		_sun->corrected_position_horizontal_coordinates = corrected_sun_position_near_horizon (_sun->position.horizontal_coordinates);
		_sun->corrected_position_cartesian_horizontal_coordinates = compute_cartesian_horizontal_coordinates (_sun->corrected_position_horizontal_coordinates);
		_sun->color_on_body = to_gl_color (compute_sun_light_color (_camera_polar_position, _sun->corrected_position_cartesian_horizontal_coordinates, _sun->atmospheric_scattering));
	}

	if (_planet && _sun)
	{
		auto const sun_altitude = _sun->position.horizontal_coordinates.altitude;
		_sun_altitude_above_horizon = sun_altitude - _planet->horizon_angle;
	}

	_sky_box_visibility = _sun_altitude_above_horizon
		? compute_sky_box_visibility (*_sun_altitude_above_horizon)
		: 1.0f;
}


void
RigidBodyPainter::setup_camera (QOpenGLPaintDevice& canvas)
{
	setup_camera_projection (canvas.size());
	setup_modelview();
	compute_camera_transform();
}


void
RigidBodyPainter::setup_camera_projection (QSize const size)
{
	glMatrixMode (GL_PROJECTION);
	_gl.load_identity();
	_gl.translate (0.0, 0.0, -1.0);
	_gl.set_hfov_perspective (size, _fov, _gl.to_opengl (1_m), _gl.to_opengl (100_km));
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
	glEnable (GL_CULL_FACE);
	glEnable (GL_DEPTH_TEST);
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
	if (_sun)
	{
		// Blend the original sun color with color as seen through the atmosphere:
		auto const qcolor = QColor::fromRgbF (_sun->color_on_body[0], _sun->color_on_body[1], _sun->color_on_body[2]);
		auto const height_factor = _planet ? _planet->camera_clamped_normalized_amsl_height : 1.0f;
		auto const atmospheric_sun_color = to_gl_color (hsl_interpolation (height_factor, qcolor, kSunQColorInSpace));

		glLightfv (kGLAtmosphericSunLight, GL_AMBIENT, atmospheric_sun_color.scaled (0.0f));
		glLightfv (kGLAtmosphericSunLight, GL_DIFFUSE, atmospheric_sun_color.scaled (0.4f));
		glLightfv (kGLAtmosphericSunLight, GL_SPECULAR, atmospheric_sun_color.scaled (0.1f));

		// When we're inside the atmosphere, the Moon light (and potentially other bodies) are too intensive when combined with SkyDome, so use darker diffuse
		// color. When going out of atmosphere, Moon becomes too dark, so use lighter color.
		auto const diffuse_scale_space = nu::renormalize (_planet->camera_clamped_normalized_amsl_height, nu::Range { 0.0f, 1.0f }, nu::Range { 0.3f, 2.0f });
		// Also update the scale to take into account night (the Moon should be brighter at night for the same reason we make it brighter in space.
		auto const diffuse_scale_night = nu::renormalize (_sky_box_visibility, nu::Range { 0.0f, 1.0f }, nu::Range { 0.3f, 2.0f });

		glLightfv (kGLCosmicSunLight, GL_AMBIENT, kSunColorInSpace.scaled (0.0f));
		glLightfv (kGLCosmicSunLight, GL_DIFFUSE, kSunColorInSpace.scaled (std::max (diffuse_scale_space, diffuse_scale_night)));
		glLightfv (kGLCosmicSunLight, GL_SPECULAR, kSunColorInSpace.scaled (0.0f));

		_gl.save_context ([&]{
			_gl.set_camera (_camera);

			if (_sun)
			{
				_gl.save_context ([&]{
					_gl.load_identity();
					_gl.set_camera_rotation_only (_camera);
					make_z_towards_the_sun (_sun->position);
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
	if (_planet && _sun)
	{
		// Sky lights:
		for (auto& sky_light: _planet->sky_lights)
		{
			_gl.save_context ([&]{
				make_z_sky_top_x_sun_azimuth (_sun->position);
				_gl.rotate_z (+sky_light.position.lon());
				_gl.rotate_y (-sky_light.position.lat());

				auto const number = sky_light.gl_number;
				auto const light_direction = to_cartesian<void> (sky_light.position); // Azimuth (lon()) should possible be negated, but the sky dome is symmetric, so this is okay.
				auto const color = _sun->atmospheric_scattering.compute_incident_light (
					{ 0_m, 0_m, _followed_polar_position.radius() },
					light_direction,
					_sun->position.cartesian_horizontal_coordinates
				);
				auto const corrected_color = sky_correction (color, _sun->position);
				auto const gl_color = to_gl_color (corrected_color);
				auto const sky_height = kAtmosphereRadius - kEarthMeanRadius;
				auto const black = GLColor (0.0f, 0.0f, 0.0f);

				glEnable (number);
				glLightfv (number, GL_AMBIENT, black);
				glLightfv (number, GL_DIFFUSE, gl_color.scaled (0.2f));
				glLightfv (number, GL_SPECULAR, black);
				glLightfv (number, GL_POSITION, GLArray { _gl.to_opengl (sky_height), 0.0f, 0.0f, 0.0f });
			});
		}
	}
}


void
RigidBodyPainter::set_sky_light_enabled (bool enabled)
{
	if (_planet)
	{
		for (auto& sky_light: _planet->sky_lights)
		{
			auto func = enabled ? &::glEnable : &::glDisable;
			func (sky_light.gl_number);
		}
	}
}


void
RigidBodyPainter::setup_feature_light()
{
	// Enable feature light:
	glLightfv (kGLFeatureLight, GL_AMBIENT, GLArray { 0.25f, 0.25f, 0.25f, 1.0f });
	glLightfv (kGLFeatureLight, GL_DIFFUSE, GLArray { 0.5f, 0.5f, 0.5f, 1.0f });
	glLightfv (kGLFeatureLight, GL_SPECULAR, GLArray { 0.9f, 0.9f, 0.9f, 1.0f });

	_gl.save_context ([&]{
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
RigidBodyPainter::enable_appropriate_lights()
{
	if (_sun)
	{
		if (_planet)
			enable_only_lights (kAtmosphericSunLight | kSkyLight);
		else
			enable_only_lights (kCosmicSunLight);
	}
	else
		enable_only_lights (kFeatureLight);
}


void
RigidBodyPainter::make_z_towards_the_sun (SunPosition const& sun_position)
{
	// Assuming we have identity rotations + camera rotations applied.

	// Rotate sun depending on UTC time:
	auto const greenwich_hour_angle = sun_position.hour_angle - _camera_polar_position.lon();
	_gl.rotate_z (-greenwich_hour_angle);

	// Rotate sun depending on time of the year:
	_gl.rotate_y (-sun_position.declination);

	// Our sun is located over the North Pole (+Z in ECEF). Rotate it to be straight over Null Island
	// that is lon/lat 0°/0° (+X in ECEF):
	_gl.rotate_y (90_deg);
}


void
RigidBodyPainter::make_z_sky_top_x_south()
{
	_gl.rotate (z_rotation<WorldSpace> (_camera_polar_position.lon()) * y_rotation<WorldSpace> (-_camera_polar_position.lat() + 90_deg));
	// Z is now sky top, X is towards azimuth 180° (true south).
}


void
RigidBodyPainter::make_z_sky_top_x_sun_azimuth (SunPosition const& sun_position)
{
	make_z_sky_top_x_south();
	_gl.rotate_z (-sun_position.horizontal_coordinates.azimuth + 180_deg);
}


void
RigidBodyPainter::paint_world (rigid_body::System const& system)
{
	_gl.save_context ([&]{
		paint_universe_and_sun();
		// The known problem with Moon painting is that the Eart (and SkyDome)
		// is always visible (in front of the Moon), even when it should be
		// obscured by the Moon. It's because GL_DEPTH_TEST is disabled when
		// painting SkyDome. Will fix later or never. Not important.
		paint_moon();
		paint_planet();
		paint_air_particles();
		paint (system);
	});
}


void
RigidBodyPainter::paint_universe_and_sun()
{
	check_sky_box();

	if (_universe && _universe->sky_box_shape)
	{
		_gl.save_context ([&]{
			auto const alpha = _sun_altitude_above_horizon
				? compute_sky_box_visibility (*_sun_altitude_above_horizon)
				: 1.0f;
			auto const x = 0.5f; // Darken the universe a bit.
			auto const color = GLColor (x, x, x, alpha);

			_universe->sky_box_shape->for_each_vertex ([color] (ShapeVertex& vertex) {
				vertex.material().gl_texture_color = color;
			});

			// Enable blending for alpha modulation:
			glEnable (GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable (GL_ALPHA_TEST);

			glFrontFace (GL_CCW);
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_LIGHTING);
			glEnable (GL_TEXTURE_2D);
			glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			_gl.set_camera_rotation_only (_camera);
			_gl.rotate (~(kScreenToNullIslandRotation * _universe->ecef_to_celestial_rotation));
			_gl.draw (*_universe->sky_box_shape);
			_gl.clear_z_buffer();
			glDisable (GL_TEXTURE_2D);
			glDisable (GL_BLEND);
		});
	}

	if (_sun)
	{
		// Sun:
		_gl.save_context ([&]{
			auto const magnification_at_0_amsl = _sun_altitude_above_horizon
				? nu::renormalize (*_sun_altitude_above_horizon, nu::Range { 0_deg, 90_deg }, nu::Range { kSunSunsetMagnification, kSunNoonMagnification })
				: 1.0f;
			// Correct for the fact that the magnification only happens inside atmosphere:
			_sun->magnification = _planet
				? 1.0f + (magnification_at_0_amsl - 1.0f) * (1.0f - _planet->camera_clamped_normalized_amsl_height)
				: 1.0f;

			glEnable (GL_BLEND);
			// Disable Z-testing so that the sun gets rendered even if it's far behind the sky dome sphere:
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable (GL_ALPHA_TEST);

			// Enable blending, to blend well the Sun with the sky:
			glDisable (GL_DEPTH_TEST);
			glDisable (GL_LIGHTING);
			// We drawing the 'inside' of the sphere with sun_shines:
			glFrontFace (GL_CW);

			// Keep translation in world/ECEF space for consistency with Moon/object rendering.
			_gl.set_camera_rotation_only (_camera);
			_gl.translate (-_camera.position());
			make_z_towards_the_sun (_sun->position);
			// Z is now direction towards the Sun:
			_gl.translate (0_m, 0_m, kSunDistance);
			_gl.save_context ([&]{
				auto const scale = _sun->magnification;
				glScalef (scale, scale, scale);
				_gl.draw (_sun->face_shape);
			});
			auto const time_dependent_angle = 360_deg * _time.in<si::Hour>() / 24.0;
			// Rotate sun shines when camera angle and simulation time changes:
			_gl.rotate (_user_camera_angles[0] - 2 * _user_camera_angles[1] + time_dependent_angle, 0, 0, 1);

			auto const sun_visibility = _sun_altitude_above_horizon
				? 1.0f - nu::square (1.0f - compute_sun_visible_surface_factor (*_sun_altitude_above_horizon))
				: 1.0f;
			auto const alpha_height_factor = _planet ? _planet->camera_clamped_normalized_amsl_height : 1.0f;

			_gl.save_context ([&]{
				_gl.additional_parameters().alpha_factor = std::clamp<float> (alpha_height_factor * sun_visibility, 0.0f, 1.0f);
				_gl.draw (_sun->shines_shape);
			});
		});
	}
}


void
RigidBodyPainter::paint_moon()
{
	if (!_sun)
		return;

	static auto const moon_material = []{
		auto material = ShapeMaterial {};
		material.gl_emission_color = GLColor { 0.0f, 0.0f, 0.0f, 1.0f };
		material.gl_ambient_color = GLColor { 0.0f, 0.0f, 0.0f, 1.0f };
		material.gl_diffuse_color = GLColor { 1.0f, 1.0f, 1.0f, 1.0f };
		material.gl_specular_color = GLColor { 1.0f, 1.0f, 1.0f, 1.0f };
		material.gl_shininess = 0.0f;
		return material;
	}();

	if (!_moon_shape)
	{
		_moon_shape = make_centered_sphere_shape ({
			.radius = kMoonRadius,
			.n_slices = 50,
			.n_stacks = 25,
			.material = moon_material,
			.setup_material = [] (ShapeMaterial& material, si::LonLat const sphere_position) {
				material.texture_position = {
					nu::renormalize (sphere_position.lon(), nu::Range { -180_deg, +180_deg }, nu::Range { 0.0f, 1.0f }),
					nu::renormalize (sphere_position.lat(), nu::Range { -90_deg, +90_deg }, nu::Range { 1.0f, 0.0f }),
				};
			},
			.texture = _planet_textures ? _planet_textures->moon : nullptr,
		});
	}

	bool const has_moon_texture = _planet_textures && _planet_textures->moon;
	auto const moon_position = planet_position() + compute_moon_position_in_ecef (_time);
	auto const direction_to_earth = (planet_position() - moon_position).normalized();
	auto const moon_rotation = alpha_beta_from_x_to (direction_to_earth);

	_gl.save_context ([&]{
		glDisable (GL_BLEND);

		if (has_moon_texture)
		{
			glEnable (GL_TEXTURE_2D);
			glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
		else
			glDisable (GL_TEXTURE_2D);

		glEnable (GL_DEPTH_TEST);
		glFrontFace (GL_CCW);
		enable_only_lights (kCosmicSunLight);
		// Textured vertices set only diffuse in GLSpace::set_texture(), so force
		// non-emissive, low-ambient material here to avoid leaking sun material state.
		glMaterialf (GL_FRONT, GL_SHININESS, 0.0f);
		glMaterialfv (GL_FRONT, GL_EMISSION, GLArray { 0.0f, 0.0f, 0.0f, 1.0f });
		glMaterialfv (GL_FRONT, GL_AMBIENT, GLArray { 0.0f, 0.0f, 0.0f, 1.0f });
		glMaterialfv (GL_FRONT, GL_SPECULAR, GLArray { 0.0f, 0.0f, 0.0f, 1.0f });

		// Keep translation in world/ECEF space: if we rotate after set_camera(),
		// camera position would get rotated together with Moon model vertices.
		_gl.set_camera_rotation_only (_camera);
		_gl.translate (moon_position - _camera.position());
		// Keep the same lunar hemisphere facing Earth.
		_gl.rotate_z (moon_rotation[0]);
		_gl.rotate_y (moon_rotation[1]);
		_gl.draw (*_moon_shape);

		glDisable (GL_TEXTURE_2D);
	});
}


void
RigidBodyPainter::paint_planet()
{
	if (!_planet)
		return;

	_gl.save_context ([&]{
		_gl.set_camera (_camera);

		// Ground:
		// TODO it would be best if there was a shader that adds the dome sphere color to the drawn feature
		if (_planet->ground_shape)
		{
			_gl.save_context ([&]{
				glFrontFace (GL_CCW);
				glDisable (GL_BLEND);
				glEnable (GL_DEPTH_TEST);
				glEnable (GL_TEXTURE_2D);
				glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				_gl.translate (planet_position());
				enable_only_lights (kCosmicSunLight);
				_gl.draw (*_planet->ground_shape);
				glDisable (GL_TEXTURE_2D);
			});
		}

		if (_planet->sky_dome_shape)
		{
			// Sky and ground dome around the observer:
			_gl.save_context ([&]{
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
				_gl.draw (*_planet->sky_dome_shape);
				glDisable (GL_BLEND);
				glEnable (GL_LIGHTING);
				glEnable (GL_DEPTH_TEST);
				glFrontFace (GL_CCW);
			});
		}
	});
}


void
RigidBodyPainter::paint_air_particles()
{
	_gl.save_context ([&]{
		enable_appropriate_lights();
		_gl.set_camera_rotation_only (_camera);
		// Trick with rotating camera and then subtracting camera position from the object is to avoid problems with low precision OpenGL floats:
		// _followed_position - _camera.position() uses doubles; but _gl.translate() internally reduces them to floats:
		_gl.translate (_followed_position - _camera.position());

		// Air 'particles' only appear if we have a planet:
		// TODO both dust size and grid size should depend on the airplane size (max bounds?)
		auto const dust_size = 2_cm;
		auto const grid_size = 5_m;
		static auto const range = 3 * grid_size;
		static auto const dust = make_centered_sphere_shape ({ .radius = dust_size, .n_slices = 3, .n_stacks = 3, .material = kWhiteMatte });

		// Figure out nearest 3D grid points.
		// Then wiggle each one pseudo-randomly.
		auto const body_pos = _followed_position;
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
					auto seed = std::seed_seq {
						static_cast<int64_t> (x.in<si::Meter>()),
						static_cast<int64_t> (y.in<si::Meter>()),
						static_cast<int64_t> (z.in<si::Meter>()),
					};
					_air_particles_prng.seed (seed);
					_gl.save_context ([&]{
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
	_gl.save_context ([&]{
		enable_appropriate_lights();
		_gl.set_camera_rotation_only (_camera);

		for (auto const& body: system.bodies())
			paint (*body, get_rendering_config (*body));

		if (_features_config.constraints_visible)
		{
			for (auto const& constraint: system.constraints())
				paint (*constraint);
		}
		else
		{
			// Even if constraints are not set to 'visible', if a constraint is hovered or focused
			// in the tree-widget, it will still be painted, for convenience.

			auto const* focused_constraint = this->focused_constraint();
			auto const* hovered_constraint = hovered<rigid_body::Constraint>();

			if (focused_constraint || hovered_constraint)
			{
				for (auto const& constraint: system.constraints())
				{
					auto const* current_constraint = constraint.get();

					if (current_constraint == focused_constraint || current_constraint == hovered_constraint)
						paint (*constraint);
				}
			}
		}

		if (_features_config.gravity_visible || _features_config.aerodynamic_forces_visible || _features_config.external_forces_visible)
			for (auto const& body: system.bodies())
				paint_forces (*body);

		if (_features_config.angular_velocities_visible)
			for (auto const& body: system.bodies())
				paint_angular_velocity (*body);

		if (_features_config.angular_momenta_visible)
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
		? rotation_reference_body->placement().body_rotation()
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
	_gl.save_context ([&]{
		transform_gl_to_center_of_mass (body);

		if (rendering.moments_of_inertia_visible)
			paint_moments_of_inertia_cuboid (body.mass_moments<BodyCOM>());

		// Body shapes are defined relative to BodyOrigin coordinates, so transform again:
		transform_gl_from_body_center_of_mass_to_origin (body);

		if (rendering.body_visible)
		{
			_gl.save_context ([&]{
				if (&body == focused_body())
					_gl.additional_parameters().color_override = GLColor::from_rgb (0x00, 0xaa, 0x7f);
				else if (hovered<rigid_body::Body>() == &body)
					_gl.additional_parameters().color_override = GLColor::from_rgb (0x00, 0xaa, 0x7f).lighter (0.5);

				glFrontFace (GL_CCW);
				_gl.draw (shape_for (body));
			});
		}
	});
}


void
RigidBodyPainter::paint (rigid_body::Constraint const& constraint)
{
	if (constraint.enabled() && !constraint.broken())
	{
		_gl.save_context ([&]{
			auto const cp = _camera.position();
			auto const& b1 = constraint.body_1();
			auto const& b2 = constraint.body_2();
			auto com1 = b1.placement().position() - cp;
			auto com2 = b2.placement().position() - cp;
			auto const is_focused = &constraint == focused_constraint();
			auto const is_hovered = hovered<rigid_body::Constraint>() == &constraint;
			auto const highlighted_constraint_color = [is_focused, is_hovered] {
				auto color = QColor (0x00, 0xaa, 0x7f);

				if (is_hovered && !is_focused)
					color = color.lighter (150);

				return color;
			};

			auto const rod_from_to = [this] (si::Length const radius, auto const& from, auto const& to, bool front_back_faces, ShapeMaterial const& material)
			{
				_gl.save_context ([&]{
					auto const diff = to - from;
					_gl.translate (from);

					auto const alpha_beta = alpha_beta_from_x_to (diff);
					_gl.rotate_z (alpha_beta[0]);
					_gl.rotate_y (alpha_beta[1] + 90_deg);

					auto const shape = make_cylinder_shape ({
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
				auto const a1 = b1.placement().rotate_to_base (hinge->hinge_precomputation().body_1_anchor());
				auto const hinge_1 = b1.placement().rotate_to_base (hinge->hinge_precomputation().body_1_hinge());
				auto const hinge_start_1 = com1 + a1;
				auto const hinge_end_1 = hinge_start_1 + hinge_1;
				auto const hinge_center = hinge_start_1 + 0.5 * hinge_1;
				auto const color = (is_focused || is_hovered)
					? highlighted_constraint_color()
					: QColor (0xff, 0x99, 0x00);
				auto const material = make_material (color);

				// Lines from COM to hinge center:
				rod_from_to (kDefaultConstraintDiameter, com1, hinge_center, false, material);
				rod_from_to (kDefaultConstraintDiameter, com2, hinge_center, false, material);
				// Hinge:
				rod_from_to (kDefaultHingeDiameter, hinge_start_1, hinge_end_1, true, material);
			}
			else if (dynamic_cast<rigid_body::FixedConstraint const*> (&constraint))
			{
				auto const color = (is_focused || is_hovered)
					? highlighted_constraint_color()
					: QColor (0xff, 0x00, 0x99);
				auto const material = make_material (color);
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
		_gl.save_context ([&]{
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
		_gl.save_context ([&]{
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
	auto const com_shape = make_center_of_mass_symbol_shape (_user_camera_translation.norm() / 150);

	_gl.save_context ([&]{
		glDisable (GL_LIGHTING);
		_gl.draw (com_shape);
		glEnable (GL_LIGHTING);
	});
}


void
RigidBodyPainter::paint_origin()
{
	auto const origin_material = make_material ({ 0xff, 0xff, 0x00 });
	auto const origin_shape = make_centered_sphere_shape ({ .radius = _user_camera_translation.norm() / 150, .n_slices = 8, .n_stacks = 4, .material = origin_material });
	_gl.draw (origin_shape);
}


void
RigidBodyPainter::paint_moments_of_inertia_cuboid (MassMoments<BodyCOM> const& mass_moments)
{
	auto const com_material = make_material ({ 0x00, 0x44, 0x99 });
	auto const com_shape = make_centered_cube_shape (mass_moments, com_material);
	glFrontFace (GL_CCW);
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

	if (_features_config.gravity_visible)
		draw_arrow (com, gfm.force() * force_to_length, make_material (gravity_color));

	if (_features_config.aerodynamic_forces_visible)
	{
		if (auto const* wing = dynamic_cast<sim::Wing const*> (&body))
		{
			if (auto const params = wing->airfoil_aerodynamic_parameters();
				params)
			{
				auto const& forces = params->forces;
				auto const& pl = wing->placement();
				auto const at = pl.rotate_translate_to_base (forces.center_of_pressure) - cp;

				draw_arrow (at, pl.rotate_to_base (forces.lift) * force_to_length, make_material (lift_color));
				draw_arrow (at, pl.rotate_to_base (forces.drag) * force_to_length, make_material (drag_color));
				draw_arrow (at, pl.rotate_to_base (forces.pitching_moment) * torque_to_length, make_material (torque_color));
			}
		}
	}

	if (_features_config.external_forces_visible)
	{
		draw_arrow (com, efm.force() * force_to_length, make_material (external_force_color));
		draw_arrow (com, efm.torque() * torque_to_length, make_material (external_torque_color));
	}
}


void
RigidBodyPainter::paint_angular_velocity (rigid_body::Body const& body)
{
	auto constexpr angular_velocity_to_length = 0.1_m / 1_radps; // TODO unhardcode
	auto const com = body.placement().position() - _camera.position();
	auto const omega = body.velocity_moments<WorldSpace>().angular_velocity();

	draw_arrow (com, omega * angular_velocity_to_length, make_material (Qt::darkMagenta));
}


void
RigidBodyPainter::paint_angular_momentum (rigid_body::Body const& body)
{
	auto constexpr angular_momentum_to_length = 0.001_m / (1_kg * 1_m2 / 1_s) / 1_rad; // TODO unhardcode
	auto const com = body.placement().position() - _camera.position();
	auto const I = body.mass_moments<BodyCOM>().inertia_tensor();
	auto const L = I * body.velocity_moments<BodyCOM>().angular_velocity();
	auto const L_world = body.placement().rotate_to_base (L);

	draw_arrow (com, L_world * angular_momentum_to_length, make_material (Qt::darkBlue));
}


void
RigidBodyPainter::draw_arrow (SpaceLength<WorldSpace> const& origin, SpaceLength<WorldSpace> const& vector, ShapeMaterial const& material)
{
	_gl.save_context ([&]{
		auto const length = abs (vector);

		if (length > 0_m)
		{
			auto constexpr scale = 2.0f;
			auto constexpr kNumFaces = 12;
			auto constexpr cone_radius = 20_mm * scale;
			auto constexpr cone_length = 50_mm * scale;
			auto constexpr radius = 5_mm * scale;
			auto const alpha_beta = alpha_beta_from_x_to (vector);

			glFrontFace (GL_CCW);
			_gl.translate (origin);
			_gl.rotate_z (alpha_beta[0]);
			_gl.rotate_y (alpha_beta[1] + 90_deg);
			_gl.draw (make_cylinder_shape ({ .length = length, .radius = radius, .num_faces = kNumFaces, .with_bottom = true, .with_top = true, .material = material }));
			_gl.translate (0_m, 0_m, length);
			_gl.draw (make_cone_shape ({ .length = cone_length, .radius = cone_radius, .num_faces = kNumFaces, .with_bottom = true, .material = material }));
		}
	});
}


SpaceVector<float, RGBSpace>
RigidBodyPainter::sky_correction (SpaceVector<float, RGBSpace> rgb, SunPosition const& sun_position) const
{
	auto constexpr altitude_threshold = 4_deg;
	auto constexpr reduce_green_to = 0.8;
	auto constexpr reduce_green_to_sqrt = std::sqrt (reduce_green_to);

	if (sun_position.hour_angle > 0_deg && sun_position.hour_angle < 180_deg)
	{
		auto const abs_altitude = abs (sun_position.horizontal_coordinates.altitude);

		if (abs_altitude < altitude_threshold)
		{
			auto const from = nu::Range { altitude_threshold, 0_deg };
			auto const to = nu::Range { 1.0, reduce_green_to_sqrt };
			auto const factor = nu::renormalize (sun_position.horizontal_coordinates.altitude, from, to);
			rgb[1] *= nu::square (factor);
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

	_gl.save_context ([&]{
		glMatrixMode (GL_PROJECTION);
		_gl.load_identity();
		_gl.translate (tx, ty, -1.0);
		_gl.set_hfov_perspective (canvas.size(), 60_deg, _gl.to_opengl (1_cm), _gl.to_opengl (10_m));

		glMatrixMode (GL_MODELVIEW);
		// ECEF basis should be always "on top":
		_gl.clear_z_buffer();
		_gl.set_camera (std::nullopt);
		// TODO Try to use set_camera instead of manually managing the matrix:
		_gl.load_identity();
		_gl.translate (0_m, 0_m, -1_m);
		_gl.rotate (_camera.body_rotation());
		paint_basis (8_cm);
	});
}


void
RigidBodyPainter::paint_basis (si::Length const length)
{
	si::Length const radius = length / 50;
	si::Length const cone_radius = length / 13;
	si::Length const cone_length = length / 5;

	auto const blue = make_material (QColor (0x11, 0x11, 0xff));
	auto const red = make_material (Qt::red);
	auto const green = make_material (Qt::green);
	auto const kNumFaces = 12;

	setup_feature_light();

	glFrontFace (GL_CCW);
	// Root ball:
	_gl.draw (make_centered_sphere_shape ({ .radius = 2 * radius, .n_slices = 8, .n_stacks = 4 }));
	// X axis:
	_gl.save_context ([&]{
		_gl.rotate_y (+90_deg);
		_gl.draw (make_cylinder_shape ({ .length = length, .radius = radius, .num_faces = kNumFaces, .material = red }));
		_gl.translate (0_m, 0_m, length);
		_gl.draw (make_cone_shape ({ .length = cone_length, .radius = cone_radius, .num_faces = kNumFaces, .with_bottom = true, .material = red }));
	});
	// Y axis:
	_gl.save_context ([&]{
		_gl.rotate_x (-90_deg);
		_gl.draw (make_cylinder_shape ({ .length = length, .radius = radius, .num_faces = kNumFaces, .material = green }));
		_gl.translate (0_m, 0_m, length);
		_gl.draw (make_cone_shape ({ .length = cone_length, .radius = cone_radius, .num_faces = kNumFaces, .with_bottom = true, .material = green }));
	});
	// Z axis:
	_gl.save_context ([&]{
		_gl.draw (make_cylinder_shape ({ .length = length, .radius = radius, .num_faces = kNumFaces, .material = blue }));
		_gl.translate (0_m, 0_m, length);
		_gl.draw (make_cone_shape ({ .length = cone_length, .radius = cone_radius, .num_faces = kNumFaces, .with_bottom = true, .material = blue }));
	});
}


void
RigidBodyPainter::compute_followed_position()
{
	if (auto const* followed_body = this->followed_body())
		_followed_position = followed_body->placement().position();
	else if (auto const* followed_group = this->followed_group())
		_followed_position = get_center_of_mass (*followed_group);
	else
		_followed_position = { 0_m, 0_m, 0_m };

	_followed_polar_position = to_polar (math::coordinate_system_cast<ECEFSpace, void, WorldSpace, void> (_followed_position - planet_position()));
}


SpaceLength<WorldSpace>
RigidBodyPainter::planet_position() const
{
	if (_planet)
		return _planet->body->placement().position();
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
RigidBodyPainter::check_texture_images()
{
	check_planet_texture_images();
	check_universe_texture_images();
}


void
RigidBodyPainter::check_planet_texture_images()
{
	if (_planet)
	{
		auto images = _planet_texture_images.lock();

		if (!images->valid())
		{
			// Start loading texture images if not yet loaded by another RigidBodyPainter:
			*images = _work_performer->submit ([]{
				return PlanetTextureImages {
					.earth = QImage ("share/images/textures/earth/earth-day-2004-07.jpg"),
					.moon = QImage ("share/images/textures/moon/lroc_color_2k.jpg"),
				};
			});
		}
	}
}


void
RigidBodyPainter::check_universe_texture_images()
{
	if (_universe)
	{
		auto images = _universe_texture_images.lock();

		if (!images->valid())
		{
			// Start loading texture images if not yet loaded by another RigidBodyPainter.
			*images = std::async (std::launch::async, [this] {
				auto neg_x_future = _work_performer->submit ([]{ return QImage ("share/images/textures/universe/nx.jpg"); });
				auto neg_y_future = _work_performer->submit ([]{ return QImage ("share/images/textures/universe/ny.jpg"); });
				auto neg_z_future = _work_performer->submit ([]{ return QImage ("share/images/textures/universe/nz.jpg"); });
				auto pos_x_future = _work_performer->submit ([]{ return QImage ("share/images/textures/universe/px.jpg"); });
				auto pos_y_future = _work_performer->submit ([]{ return QImage ("share/images/textures/universe/py.jpg"); });
				auto pos_z_future = _work_performer->submit ([]{ return QImage ("share/images/textures/universe/pz.jpg"); });

				return UniverseTextureImages {
					.universe_neg_x	= neg_x_future.get(),
					.universe_neg_y	= neg_y_future.get(),
					.universe_neg_z	= neg_z_future.get(),
					.universe_pos_x	= pos_x_future.get(),
					.universe_pos_y	= pos_y_future.get(),
					.universe_pos_z	= pos_z_future.get(),
				};
			});
		}
	}
}


void
RigidBodyPainter::check_textures()
{
	check_planet_textures();
	check_universe_textures();
}


void
RigidBodyPainter::check_planet_textures()
{
	if (_planet && !_planet_textures)
	{
		auto images_accessor = _planet_texture_images.lock();

		if (nu::valid_and_ready (*images_accessor))
		{
			auto const& images = images_accessor->get();

			_planet_textures = PlanetTextures {
				.earth = make_texture (images.earth),
				.moon = make_texture (images.moon),
			};

			// Reset the shape so it's recalculated with texture next time
			// instead of basic non-textured ball:
			_moon_shape.reset();

			// Reload SkyDome to include the Earth texture:
			_planet->sky_dome_shape.reset();
		}
	}
}


void
RigidBodyPainter::check_universe_textures()
{
	if (_universe && !_universe_textures)
	{
		auto images_accessor = _universe_texture_images.lock();

		if (nu::valid_and_ready (*images_accessor))
		{
			auto const& images = images_accessor->get();

			_universe_textures = UniverseTextures {
				.universe_neg_x = make_texture (images.universe_neg_x),
				.universe_neg_y = make_texture (images.universe_neg_y),
				.universe_neg_z = make_texture (images.universe_neg_z),
				.universe_pos_x = make_texture (images.universe_pos_x),
				.universe_pos_y = make_texture (images.universe_pos_y),
				.universe_pos_z = make_texture (images.universe_pos_z),
			};
		}
	}
}


void
RigidBodyPainter::check_sky_dome_and_ground_shape()
{
	if (_planet)
	{
		if (!_planet->sky_dome_shape)
			_planet->sky_dome_shape = compute_sky_dome_shape();

		if (!_planet->ground_shape && _planet_textures)
			_planet->ground_shape = compute_ground_shape (_camera_polar_position, kEarthMeanRadius, _planet_textures->earth);
	}
}


Shape
RigidBodyPainter::compute_sky_dome_shape()
{
	_camera_position_for_sky_dome = _camera.position();

	if (_planet && _sun)
	{
		// With alpha 1.0 and being in space, the sky is a bit too bright.
		// Tone it down a bit.
		auto const sky_alpha = 1.0f - 0.1f * _planet->camera_clamped_normalized_amsl_height;

		return xf::compute_sky_dome_shape ({
			.atmospheric_scattering = _sun->atmospheric_scattering,
			.observer_position = _camera_polar_position,
			.sun_position = _sun->corrected_position_horizontal_coordinates,
			.earth_radius = kEarthMeanRadius,
			.earth_texture = _planet_textures ? _planet_textures->earth : nullptr,
			.sky_alpha = sky_alpha,
		}, &*_work_performer);
		// TODO apply sky_correction to vertices' materials
	}
	else
		return {};
}


void
RigidBodyPainter::check_sky_box()
{
	if (_universe && !_universe->sky_box_shape && _universe_textures)
	{
		_universe->sky_box_shape = make_sky_box ({
			.edge_length = 1000_m,
			.material = kWhiteMatte,
			.texture_neg_x = _universe_textures->universe_neg_x,
			.texture_neg_y = _universe_textures->universe_neg_y,
			.texture_neg_z = _universe_textures->universe_neg_z,
			.texture_pos_x = _universe_textures->universe_pos_x,
			.texture_pos_y = _universe_textures->universe_pos_y,
			.texture_pos_z = _universe_textures->universe_pos_z,
		});
	}

	// Universe sky box rotation:
	auto const julian_date = unix_time_to_julian_date (_time);
	_universe->ecef_to_celestial_rotation = compute_ecef_to_celestial_rotation (julian_date);
}


float
RigidBodyPainter::compute_sky_box_visibility (si::Angle const sun_altitude_above_horizon) const
{
	if (_planet)
	{
		// Make the universe sky box visible when high above the surface, but also at night:
		auto const night_time_visibility = nu::renormalize (sun_altitude_above_horizon, nu::Range { -3_deg, -12_deg }, nu::Range { 0.0f, 0.5f });
		auto const altitude_visibility = nu::renormalize (_planet->camera_normalized_amsl_height, nu::Range { 0.8f, 1.5f }, nu::Range { 0.0f, 1.0f });
		return std::clamp (std::max (altitude_visibility, night_time_visibility), 0.0f, 1.0f);
	}
	else
		return 1.0f;
}


void
RigidBodyPainter::compute_camera_transform()
{
	auto const* followed_group = this->followed_group();
	auto const* followed_body = this->followed_body();
	auto camera_mode = _camera_mode;

	// When CockpitView is requested but nothing is set to follow, fall back to ChaseView:
	if (_camera_mode == CockpitView && std::holds_alternative<std::monostate> (_followed))
		camera_mode = ChaseView;

	switch (camera_mode)
	{
		case CockpitView:
		{
			auto rotation = RotationQuaternion<WorldSpace> (math::identity);

			if (followed_group)
				if (auto const* rotation_reference_body = followed_group->rotation_reference_body())
					followed_body = rotation_reference_body;

			if (followed_body)
			{
				auto const base_rotation = math::coordinate_system_cast<WorldSpace, WorldSpace, BodyCOM, WorldSpace> (followed_body->placement().base_rotation());
				// Make an exception if we're following the planet body: we don't want to use aircraft coordinates
				// for the planet, because it's unnatural:
				if (_planet && _planet->body == followed_body)
					rotation = _user_camera_rotation * kScreenToNullIslandRotation * base_rotation;
				else
					rotation = _user_camera_rotation * kAircraftToBehindViewRotation * base_rotation;
			}

			_camera.set_body_rotation (rotation);
		}
		break;

		case ChaseView:
		{
			auto const rotation = _user_camera_rotation * gravity_down_rotation (_followed_polar_position) * kScreenToNullIslandRotation;
			_camera.set_body_rotation (rotation);
		}
		break;

		case RCPilotView:
			// TODO unimplemented yet
			// TODO use _requested_camera_polar_position
			_camera.set_body_rotation (_user_camera_rotation);
			break;

		case FixedView:
			// TODO unimplemented yet
			// TODO use _requested_camera_polar_position
			_camera.set_body_rotation (_user_camera_rotation);
			break;
	}

	_camera.set_position (_followed_position - planet_position() + _camera.base_rotation() * _user_camera_translation);
	_camera_polar_position = to_polar (_camera.position());

	fix_camera_position();

	if (_planet)
	{
		if (abs (_camera_position_for_sky_dome - _camera.position()) > 10_m)
		{
			_planet->sky_dome_shape.reset();
			_planet->ground_shape.reset();
		}
	}

	if (_camera_position_callback)
		_camera_position_callback (_camera.position());
}


void
RigidBodyPainter::fix_camera_position()
{
	if (_planet && _planet->body != followed_body() && _camera_polar_position.radius() < kEarthMeanRadius)
	{
		// Just a bit above the ground to ensure the earth surface is properly drawn:
		_camera_polar_position.radius() = kEarthMeanRadius + 10_m;
		_camera.set_position (to_cartesian<WorldSpace> (_camera_polar_position));
	}
}


HorizontalCoordinates
RigidBodyPainter::corrected_sun_position_near_horizon (HorizontalCoordinates orig) const
{
	if (_planet && _sun)
	{
		auto const R = _sun->magnification * kSunFaceAngularRadius;
		auto alt = orig.altitude - _planet->horizon_angle;

		if (alt < R)
			alt = 0.5 * (alt + R);

		orig.altitude = alt + _planet->horizon_angle;
	}

	return orig;
}


std::shared_ptr<QOpenGLTexture>
RigidBodyPainter::make_texture (QImage const& image)
{
	auto texture = std::make_shared<QOpenGLTexture> (QOpenGLTexture::Target2D);
	texture->setData (image);
	texture->setWrapMode (QOpenGLTexture::DirectionS, QOpenGLTexture::Repeat);
	texture->setWrapMode (QOpenGLTexture::DirectionT, QOpenGLTexture::MirroredRepeat);
	texture->setMinificationFilter (QOpenGLTexture::LinearMipMapLinear);
	texture->setMagnificationFilter (QOpenGLTexture::Nearest);
	return texture;
}


Shape
RigidBodyPainter::make_sun_shines_shape()
{
	auto sun_shines = make_centered_sphere_shape ({
		.radius = kSunDistance,
		.n_slices = 9,
		.n_stacks = 36,
		.v_range = { 0_deg, 90_deg },
		.material = kWhiteMatte,
		.setup_material = [=] (ShapeMaterial& material, si::LonLat const position) {
			auto const actual_radius = 0.02f;
			float const norm = renormalize<si::Angle> (position.lat(), nu::Range { 0_deg, 90_deg }, nu::Range { 0.0f, 1.0f });
			float const alpha = std::pow (norm + actual_radius, 6.0f);
			material.gl_emission_color = { 1.0f, 1.0f, 1.0f, alpha };
		},
	});
	negate_normals (sun_shines);
	return sun_shines;
}


[[nodiscard]]
Shape const&
RigidBodyPainter::shape_for (rigid_body::Body const& body)
{
	if (auto const& shape = body.shape())
		return *shape;
	else
	{
		auto& config = get_rendering_config (body);

		if (!config.default_body_shape)
			config.default_body_shape = make_centered_cube_shape (body.mass_moments<BodyCOM>());

		return *config.default_body_shape;
	}
}

} // namespace xf
