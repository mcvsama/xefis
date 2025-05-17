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
#include <xefis/support/earth/earth.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/simulation/shapes/various_shapes.h>
#include <xefis/support/ui/gl_color.h>
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
	Range<si::Angle>	longitude;
	Range<si::Angle>	latitude;
};


/**
 * Calculate an angle at which a point on a circle is visible when viewed not from a center of the circle, but from some distance from its center.
 * This works only for distance_from_center <= radius
 */
[[nodiscard]]
constexpr si::Angle
calculate_angle_from_offset_viewpoint (si::Angle const origin_angle, si::Length const radius, si::Length const distance_from_center)
{
	return 1_rad * atan2 (radius * sin (origin_angle) - distance_from_center, radius * cos (origin_angle));
}


/**
 * Inverse for calculate_angle_from_offset_viewpoint() (assuming distance_from_center and radius are the same as for view_angle()).
 * If distance_from_center > radius, it's possible that at some angles there will be no intersection of viewing direction
 * and the circle's radius. In such case function returns NaN.
 */
[[nodiscard]]
constexpr si::Angle
calculate_angle_from_center_viewpoint (si::Angle const view_angle, si::Length const radius, si::Length const distance_from_center)
{
	auto const arg = std::clamp (distance_from_center / radius * cos (view_angle), -1.0, +1.0);
	auto const k = 1_rad * asin (arg);
	return 180_deg - k + view_angle;
}


[[nodiscard]]
LonLatRanges
calculate_visible_lon_lat_ranges (si::Angle const horizon_angle, si::Length const earth_radius, si::LonLatRadius<> const observer_position)
{
	auto longitude_range = Range<si::Angle>();

	auto const alpha = -horizon_angle;

	auto const observer_position_in_ecef = to_cartesian (observer_position);
	auto const pole_visible = abs (observer_position_in_ecef.z()) > earth_radius;

	if (pole_visible)
		longitude_range = Range<si::Angle> { -180_deg, +180_deg };
	else
	{
		auto const x = (cos (alpha) - square (sin (observer_position.lat()))) / square (cos (observer_position.lat()));
		auto const b = 1_rad * std::acos (std::clamp (x, -1.0, +1.0));
		longitude_range = Range (observer_position.lon() - b, observer_position.lon() + b);
	}

	auto const latitude_range = Range (std::max<si::Angle> (-90_deg, observer_position.lat() - alpha), std::min<si::Angle> (90_deg, observer_position.lat() + alpha));

	return {
		.longitude = longitude_range,
		.latitude = latitude_range,
	};
}


[[nodiscard]]
SlicesStacks
calculate_ground_slices_and_stacks (si::Angle const horizon_angle, si::Length const earth_radius, si::LonLatRadius<> const observer_position)
{
	SlicesStacks result;
	// Determine required ranges of longitude and latitude:
	auto const ranges = calculate_visible_lon_lat_ranges (horizon_angle, earth_radius, observer_position);

	// Longitude:
	{
		auto const n_slices = 80u;
		auto const delta = ranges.longitude.extent() / n_slices;
		result.slice_angles.reserve (n_slices + 2);

		for (auto longitude = ranges.longitude.min(); longitude <= ranges.longitude.max(); longitude += delta)
			result.slice_angles.push_back (longitude);
	}

	// Latitude:
	{
		auto const n_stacks = 20u;
		auto const delta = ranges.latitude.extent() / n_stacks;
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
calculate_dome_slices_and_stacks (HorizontalCoordinates const sun_position, si::Angle const horizon_angle)
{
	SlicesStacks result;

	// Sky longitude:
	{
		auto constexpr sun_vicinity_slices = 13u;
		auto constexpr rest_slices = 50u;

		result.slice_angles.reserve (sun_vicinity_slices + rest_slices + 1);

		auto const sun_longitude = 180_deg - sun_position.azimuth;
		auto const sun_vicinity = Range { sun_longitude - 20_deg, sun_longitude + 20_deg };
		auto const sun_vicinity_delta = sun_vicinity.extent() / sun_vicinity_slices;
		auto const rest_range = 360_deg - sun_vicinity.extent();
		auto const rest_delta = rest_range / rest_slices;

		auto longitude = sun_vicinity.min();

		for (auto i = 0u; i < sun_vicinity_slices; ++i, longitude += sun_vicinity_delta)
			result.slice_angles.push_back (longitude);

		for (auto i = 0u; i < rest_slices; ++i, longitude += rest_delta)
			result.slice_angles.push_back (longitude);

		// Complete the circle where we started:
		result.slice_angles.push_back (result.slice_angles[0]);
	}

	// Latitude:
	{
		auto const horizon_epsilon = 0.001_deg;
		auto const n_ground_stacks = 12u;
		auto const n_sky_stacks = 30u;

		result.stack_angles.reserve (n_ground_stacks + n_sky_stacks);

		// Ground:
		{
			auto const step = 1.0f / n_ground_stacks;

			for (auto f = 0.0f; f < 1.0f; )
			{
				auto const curved = f * f * f * f;
				auto const latitude = neutrino::renormalize (curved, Range { 0.0f, 1.0f }, Range<si::Angle> { horizon_angle - horizon_epsilon, -90_deg });
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
			auto const power_factor = neutrino::renormalize (horizon_angle, Range<si::Angle> { 0_deg, -90_deg }, Range { 3.0, 6.0 });
			auto const limit = 1 - horizon_angle / -120_deg; // 120 instead of 90 is to avoid artifacts when there are too few stacks.

			for (auto f = 0.0f; f < limit; )
			{
				auto const curved = std::pow (f, power_factor);
				auto const latitude = neutrino::renormalize (curved, Range { 0.0f, 1.0f }, Range<si::Angle> { horizon_angle + horizon_epsilon, +90_deg });
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
calculate_sun_position (si::LonLat const observer_position, si::Time const time)
{
	// Reposition Sun according to time:
	auto const days_since_J2000 = unix_time_to_days_since_J2000 (time);
	auto const sun_ecliptic_position = calculate_sun_ecliptic_position (days_since_J2000);
	auto const sun_equatorial_position = calculate_sun_equatorial_position (sun_ecliptic_position.longitude, days_since_J2000);
	// Since equatorial coordinate system doesn't rotate with Earth, we need to take
	// that rotation into account manually (calculate hour-angle and rotate the sun again):
	auto const local_sidereal_time = unix_time_to_local_sidereal_time (time, observer_position.lon());
	auto const hour_angle = calculate_hour_angle (local_sidereal_time, sun_equatorial_position.right_ascension);
	auto const declination = sun_equatorial_position.declination;
	auto const horizontal_coordinates = calculate_sun_horizontal_position (declination, observer_position.lat(), hour_angle);
	// Azimuth 0° is North, Hour angle 0° is Noon, so add 180_deg. And the direction is opposite, so negate.
	auto const cartesian_horizontal_coordinates = calculate_cartesian_horizontal_coordinates (horizontal_coordinates);

	return {
		.hour_angle = hour_angle,
		.declination = declination,
		.horizontal_coordinates = horizontal_coordinates,
		.cartesian_horizontal_coordinates = cartesian_horizontal_coordinates,
	};
}


SpaceVector<float, RGBSpace>
calculate_sun_light_color (si::LonLatRadius<> const observer_position, SpaceVector<double> const sun_position, AtmosphericScattering const& atmospheric_scattering)
{
	return atmospheric_scattering.calculate_incident_light ({ 0_m, 0_m, observer_position.radius() }, sun_position, sun_position);
}


Shape
calculate_ground_shape (si::LonLatRadius<> const observer_position,
						si::Length const earth_radius,
						std::shared_ptr<QOpenGLTexture> earth_texture)
{
	if (auto const horizon_angle = calculate_horizon_angle (earth_radius, observer_position.radius());
		isfinite (horizon_angle))
	{
		auto const ss = calculate_ground_slices_and_stacks (horizon_angle, earth_radius, observer_position);

		if (ss.slice_angles.empty())
			return {};

		return make_centered_irregular_sphere_shape ({
			.radius = earth_radius,
			.slice_angles = ss.slice_angles,
			.stack_angles = ss.stack_angles,
			.material = kBlackMatte,
			.setup_material = [&] (ShapeMaterial& material, si::LonLat const sphere_position) {
				material.texture_position = {
					neutrino::renormalize (sphere_position.lon(), Range { -180_deg, +180_deg }, Range { 0.0f, 1.0f }),
					neutrino::renormalize (sphere_position.lat(), Range { -90_deg, +90_deg }, Range { 1.0f, 0.0f }),
				};
			},
			.texture = earth_texture,
		});
	}
	else
		return {};
}


Shape
calculate_sky_dome_shape (SkyDomeParameters const& p, neutrino::WorkPerformer* const work_performer)
{
	auto horizon_angle = calculate_horizon_angle (p.earth_radius, p.observer_position.radius());

	// Still draw sky if horizon_angle is nan (assume it's 0° then):
	if (!isfinite (horizon_angle))
		horizon_angle = 0_deg;

	auto const ss = calculate_dome_slices_and_stacks (p.sun_position, horizon_angle);
	auto const cartesian_sun_position = calculate_cartesian_horizontal_coordinates (p.sun_position);
	return make_centered_irregular_sphere_shape ({
		.radius = p.earth_radius, // TODO 10 mm around the camera
		.slice_angles = ss.slice_angles,
		.stack_angles = ss.stack_angles,
		.material = kBlackMatte,
		.setup_material = [&] (ShapeMaterial& material, si::LonLat const sphere_position, WaitGroup::WorkToken&& work_token) {
			auto calculate = [=, &material, &p, work_token = std::move (work_token)] {
				if (sphere_position.lat() >= horizon_angle)
				{
					// Sky:
					auto const polar_ray_direction = si::LonLat (sphere_position.lon(), sphere_position.lat());
					auto const ray_direction = to_cartesian<void> (polar_ray_direction);
					auto const color = p.atmospheric_scattering.calculate_incident_light(
						{ 0_m, 0_m, p.observer_position.radius() },
						ray_direction,
						cartesian_sun_position
					);
					material.gl_emission_color = to_gl_color (color);
				}
				else
				{
					// Ground haze:
					auto const cartesian_observer_position = SpaceLength (0_m, 0_m, p.observer_position.radius());
					auto const polar_ray_direction = si::LonLat (sphere_position.lon(), sphere_position.lat());
					auto const ray_direction = to_cartesian<void> (polar_ray_direction);
					si::Length max_distance = std::numeric_limits<si::Length>::infinity();

					if (auto const intersections = p.atmospheric_scattering.ray_sphere_intersections (cartesian_observer_position, ray_direction, p.earth_radius))
					{
						// If the ray intersects the Earth's sphere (planetary body) and the intersection is ahead of the camera:
						if (intersections->second > 0_m)
							// Limit the ray's effective length to the first intersection point.
							max_distance = std::max (0_m, intersections->first);
					}

					auto const color = p.atmospheric_scattering.calculate_incident_light(
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
				work_performer->submit (std::move (calculate));
			else
				calculate();
		},
	});
}

} // namespace xf

