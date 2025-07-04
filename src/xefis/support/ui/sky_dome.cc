/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
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
#include "sky_dome.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/shapes/various_shapes.h>
#include <xefis/support/ui/gl_color.h>
#include <xefis/support/universe/earth/utility.h>
#include <xefis/support/universe/julian_calendar.h>

// Standard:
#include <cstddef>
#include <atomic>
#include <functional>
#include <memory>


namespace xf {

struct SlicesStacks
{
	std::vector<si::Angle>	slice_angles;
	std::vector<si::Angle>	stack_angles;
};


struct LonLatRanges
{
	nu::Range<si::Angle>	longitude;
	nu::Range<si::Angle>	latitude;
};


/**
 * Calculate an angle at which a point on a circle is visible when viewed not from a center of the circle, but from some distance from its center.
 * This works only for distance_from_center <= radius
 */
[[nodiscard]]
constexpr si::Angle
compute_angle_from_offset_viewpoint (si::Angle const origin_angle, si::Length const radius, si::Length const distance_from_center)
{
	return 1_rad * atan2 (radius * sin (origin_angle) - distance_from_center, radius * cos (origin_angle));
}


/**
 * Inverse for compute_angle_from_offset_viewpoint() (assuming distance_from_center and radius are the same as for view_angle()).
 * If distance_from_center > radius, it's possible that at some angles there will be no intersection of viewing direction
 * and the circle's radius. In such case function returns NaN.
 */
[[nodiscard]]
constexpr si::Angle
compute_angle_from_center_viewpoint (si::Angle const view_angle, si::Length const radius, si::Length const distance_from_center)
{
	auto const arg = distance_from_center / radius * cos (view_angle);
	auto const k = 1_rad * asin (std::clamp (arg, -1.0, +1.0));
	return 180_deg - k + view_angle;
}


[[nodiscard]]
LonLatRanges
compute_visible_lon_lat_ranges (si::Angle const horizon_angle, si::Length const earth_radius, si::LonLatRadius<> const observer_position)
{
	auto longitude_range = nu::Range<si::Angle>();

	auto const alpha = -horizon_angle;

	auto const observer_position_in_ecef = to_cartesian (observer_position);
	auto const pole_visible = abs (observer_position_in_ecef.z()) > earth_radius;

	if (pole_visible)
		longitude_range = nu::Range<si::Angle> { -180_deg, +180_deg };
	else
	{
		auto const x = (cos (alpha) - nu::square (sin (observer_position.lat()))) / nu::square (cos (observer_position.lat()));
		auto const b = 1_rad * std::acos (std::clamp (x, -1.0, +1.0));
		longitude_range = nu::Range (observer_position.lon() - b, observer_position.lon() + b);
	}

	auto const latitude_range = nu::Range (std::max<si::Angle> (-90_deg, observer_position.lat() - alpha), std::min<si::Angle> (90_deg, observer_position.lat() + alpha));

	return {
		.longitude = longitude_range,
		.latitude = latitude_range,
	};
}


[[nodiscard]]
SlicesStacks
compute_ground_slices_and_stacks (si::Angle const horizon_angle, si::Length const earth_radius, si::LonLatRadius<> const observer_position)
{
	SlicesStacks result;
	// Determine required ranges of longitude and latitude:
	auto const ranges = compute_visible_lon_lat_ranges (horizon_angle, earth_radius, observer_position);

	// Longitude:
	{
		auto const n_slices = 80u;
		// Ensure delta is non-zero, to avoid infinite loop later:
		auto const delta = std::max<si::Angle> (ranges.longitude.extent() / n_slices, 1e-6_deg);
		result.slice_angles.reserve (n_slices + 2);

		for (auto longitude = ranges.longitude.min(); longitude <= ranges.longitude.max(); longitude += delta)
			result.slice_angles.push_back (longitude);
	}

	// Latitude:
	{
		auto const n_stacks = 40u;
		// Ensure delta is non-zero, to avoid infinite loop later:
		auto const delta = std::max<si::Angle> (ranges.latitude.extent() / n_stacks, 1e-6_deg);
		result.stack_angles.reserve (n_stacks + 4);

		result.stack_angles.push_back (-90_deg);

		for (auto latitude = ranges.latitude.min(); latitude <= ranges.latitude.max(); latitude += delta)
			result.stack_angles.push_back (latitude);

		result.stack_angles.push_back (+90_deg);
	}

	return result;
}


[[nodiscard]]
SlicesStacks
compute_dome_slices_and_stacks (si::Angle const horizon_angle)
{
	SlicesStacks result;

	// Sky longitude:
	{
		auto constexpr sun_vicinity_lr = 20_deg;
		auto constexpr sun_vicinity_delta = 3_deg;
		auto constexpr far_delta = 4_deg;
		auto constexpr n_sun_vicinity_slices = static_cast<std::size_t> (std::ceil (sun_vicinity_lr / sun_vicinity_delta));
		auto constexpr n_far_slices = static_cast<std::size_t> (std::ceil ((180_deg - sun_vicinity_lr) / far_delta));

		result.slice_angles.reserve (n_sun_vicinity_slices + n_far_slices + 2); // +2 for safe margin.

		// Assume that sun is always at position 0°. The shape will have to be later rotated to actual sun position (180° - azimuth).
		// Also create only slices in range (0, 180°), because the shape will be symmetric and we'll use the symmetric_0_180 = true option
		// in IrregularSphereShapeParameters.

		auto longitude = 0_deg;

		for (; longitude < sun_vicinity_lr; longitude += sun_vicinity_delta)
			result.slice_angles.push_back (longitude);

		for (; longitude < 180_deg; longitude += far_delta)
			result.slice_angles.push_back (longitude);

		result.slice_angles.push_back (180_deg);
	}

	// Latitude:
	{
		auto const horizon_epsilon = 0.001_deg;
		auto const n_ground_stacks = 20u;
		auto const n_sky_stacks = 30u;

		result.stack_angles.reserve (n_ground_stacks + n_sky_stacks);

		// Ground:
		{
			auto const step = 1.0f / n_ground_stacks;

			for (auto f = 0.0f; f < 1.0f; )
			{
				auto const curved = f * f * f;
				auto const latitude = nu::renormalize (curved, nu::Range { 0.0f, 1.0f }, nu::Range<si::Angle> { horizon_angle - horizon_epsilon, -90_deg });
				result.stack_angles.push_back (latitude);
				f += step;
			}

			// Bottom of the dome:
			result.stack_angles.push_back (-90_deg);

			// Latitudes must be in ascending order:
			std::ranges::reverse (result.stack_angles);
		}

		// Sky:
		{
			auto const step = 1.0f / n_sky_stacks;
			auto const power_factor = nu::renormalize (horizon_angle, nu::Range<si::Angle> { 0_deg, -90_deg }, nu::Range { 3.0, 6.0 });
			auto const limit = 1 - horizon_angle / -120_deg; // 120 instead of 90 is to avoid artifacts when there are too few stacks.

			for (auto f = 0.0f; f < limit; )
			{
				auto const curved = std::pow (f, power_factor);
				auto const latitude = nu::renormalize (curved, nu::Range { 0.0f, 1.0f }, nu::Range<si::Angle> { horizon_angle + horizon_epsilon, +90_deg });
				result.stack_angles.push_back (latitude);
				f += step;
			}

			// Top of the dome:
			result.stack_angles.push_back (90_deg);
		}
	}

	return result;
}


SunPosition
compute_sun_position (si::LonLat const observer_position, si::Time const time)
{
	// Reposition Sun according to time:
	auto const days_since_J2000 = unix_time_to_days_since_J2000 (time);
	auto const sun_ecliptic_position = compute_sun_ecliptic_position (days_since_J2000);
	auto const sun_equatorial_position = compute_sun_equatorial_position (sun_ecliptic_position.longitude, days_since_J2000);
	// Since equatorial coordinate system doesn't rotate with Earth, we need to take
	// that rotation into account manually (compute hour-angle and rotate the sun again):
	auto const local_sidereal_time = unix_time_to_local_sidereal_time (time, observer_position.lon());
	auto const hour_angle = compute_hour_angle (local_sidereal_time, sun_equatorial_position.right_ascension);
	auto const declination = sun_equatorial_position.declination;
	auto const horizontal_coordinates = compute_sun_horizontal_position (declination, observer_position.lat(), hour_angle);
	// Azimuth 0° is North, Hour angle 0° is Noon, so add 180_deg. And the direction is opposite, so negate.
	auto const cartesian_horizontal_coordinates = compute_cartesian_horizontal_coordinates (horizontal_coordinates);

	return {
		.hour_angle = hour_angle,
		.declination = declination,
		.horizontal_coordinates = horizontal_coordinates,
		.cartesian_horizontal_coordinates = cartesian_horizontal_coordinates,
	};
}


SpaceVector<float, RGBSpace>
compute_sun_light_color (si::LonLatRadius<> const observer_position, SpaceVector<double> const sun_position, AtmosphericScattering const& atmospheric_scattering)
{
	return atmospheric_scattering.compute_incident_light ({ 0_m, 0_m, observer_position.radius() }, sun_position, sun_position);
}


Shape
compute_ground_shape (si::LonLatRadius<> const observer_position,
					  si::Length const earth_radius,
					  std::shared_ptr<QOpenGLTexture> earth_texture)
{
	if (auto const horizon_angle = compute_horizon_angle (earth_radius, observer_position.radius());
		isfinite (horizon_angle))
	{
		auto const ss = compute_ground_slices_and_stacks (horizon_angle, earth_radius, observer_position);

		if (ss.slice_angles.empty())
			return {};

		return make_centered_irregular_sphere_shape ({
			.radius = earth_radius,
			.slice_angles = ss.slice_angles,
			.stack_angles = ss.stack_angles,
			.material = kBlackMatte,
			.texture = earth_texture,
			.setup_material = [&] (ShapeMaterial& material, si::LonLat const sphere_position) {
				material.texture_position = {
					nu::renormalize (sphere_position.lon(), nu::Range { -180_deg, +180_deg }, nu::Range { 0.0f, 1.0f }),
					nu::renormalize (sphere_position.lat(), nu::Range { -90_deg, +90_deg }, nu::Range { 1.0f, 0.0f }),
				};
			},
		});
	}
	else
		return {};
}


Shape
compute_sky_dome_shape (SkyDomeParameters const& p, nu::WorkPerformer* const work_performer)
{
	auto horizon_angle = compute_horizon_angle (p.earth_radius, p.observer_position.radius());

	// Still draw sky if horizon_angle is nan (assume it's 0° then):
	if (!isfinite (horizon_angle))
		horizon_angle = 0_deg;

	auto const ss = compute_dome_slices_and_stacks (horizon_angle);
	auto const cartesian_sun_position = compute_cartesian_horizontal_coordinates (p.sun_position);
	auto shape = make_centered_irregular_sphere_shape ({
		.radius = p.earth_radius, // TODO 10 mm around the camera
		.slice_angles = ss.slice_angles,
		.stack_angles = ss.stack_angles,
		.material = kBlackMatte,
		.symmetric_0_180 = true,
		.setup_material = [&] (ShapeMaterial& material, si::LonLat const sphere_position, nu::WaitGroup::WorkToken&& work_token) {
			// The shape originally assumes that the sun is always at 0° (more dense net is around 0°).
			// This needs a correction when used with AtmosphericScattering so that the sun is also always at 0° to match the mesh:
			auto const sky_position = si::LonLat (sphere_position.lon() + 180_deg - p.sun_position.azimuth, sphere_position.lat());
			auto compute = [=, &material, &p, work_token = std::move (work_token)] {
				if (sky_position.lat() >= horizon_angle)
				{
					// Sky:
					auto const polar_ray_direction = si::LonLat (sky_position.lon(), sky_position.lat());
					auto const ray_direction = to_cartesian<void> (polar_ray_direction);
					auto const color = p.atmospheric_scattering.compute_incident_light(
						{ 0_m, 0_m, p.observer_position.radius() },
						ray_direction,
						cartesian_sun_position
					);
					material.gl_emission_color = to_gl_color (color);
					material.gl_emission_color[3] = p.sky_alpha;
				}
				else
				{
					// Ground haze:
					auto const cartesian_observer_position = SpaceLength (0_m, 0_m, p.observer_position.radius());
					auto const polar_ray_direction = si::LonLat (sky_position.lon(), sky_position.lat());
					auto const ray_direction = to_cartesian<void> (polar_ray_direction);
					si::Length max_distance = std::numeric_limits<si::Length>::infinity();

					if (auto const intersections = p.atmospheric_scattering.ray_sphere_intersections (cartesian_observer_position, ray_direction, p.earth_radius))
					{
						// If the ray intersects the Earth's sphere (planetary body) and the intersection is ahead of the camera:
						if (intersections->second > 0_m)
							// Limit the ray's effective length to the first intersection point.
							max_distance = std::max (0_m, intersections->first);
					}

					auto const color = p.atmospheric_scattering.compute_incident_light(
						cartesian_observer_position,
						ray_direction,
						cartesian_sun_position,
						0_m,
						max_distance
					);
					material.gl_emission_color = to_gl_color (color);
					material.gl_emission_color[3] = p.ground_haze_alpha;
				}
			};

			if (work_performer)
				work_performer->submit (std::move (compute));
			else
				compute();
		},
	});

	// Since the shape assumed sun always at 0° in its spherical coordinates, we need to actually rotate it now to match the real position of the sun:
	shape.rotate (z_rotation<BodyOrigin> (180_deg - p.sun_position.azimuth));

	return shape;
}

} // namespace xf

