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
#include <xefis/support/simulation/rigid_body/various_shapes.h>
#include <xefis/support/ui/gl_color.h>

// Standard:
#include <cstddef>
#include <atomic>
#include <memory>


namespace xf {

struct SlicesStacks
{
	std::vector<si::Angle>	slice_angles;
	std::vector<si::Angle>	stack_angles;
};


/**
 * Calculate angle at which the horizon is seen at given distance from sphere of a given radius.
 * At infinite distance the result is 90°. At zero distance from sphere's tangent, it's 0°.
 * `distance_from_center` must be >= `sphere_radius` or you'll get NaNs.
 */
[[nodiscard]]
constexpr si::Angle
calculate_horizon_angle (si::Length const sphere_radius, si::Length const distance_from_center)
{
	return -1_rad * acos (sphere_radius / distance_from_center);
}


/**
 * Calculate an angle at which a point on a circle is visible when viewed not from a center of the circle, but from some distance from its center.
 * FIXME This works only for distance_from_center <= radius
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
calculate_angle_from_origin_viewpoint (si::Angle const view_angle, si::Length const radius, si::Length const distance_from_center)
{
	if (distance_from_center <= radius)
		return 1_rad * asin (distance_from_center * cos (view_angle) / radius) + view_angle;
	else
		return 1_rad * asin (distance_from_center * cos (view_angle) / radius) - view_angle;
}


[[nodiscard]]
SkyDome::SunPosition
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
	auto const cartesian_coordinates = to_cartesian<void> (si::LonLat (-horizontal_coordinates.azimuth + 180_deg, horizontal_coordinates.altitude));

	return {
		.hour_angle = hour_angle,
		.declination = declination,
		.horizontal_coordinates = horizontal_coordinates,
		.cartesian_coordinates = cartesian_coordinates,
	};
}


[[nodiscard]]
SlicesStacks
calculate_sky_slices_and_stacks (HorizontalCoordinates const sun_position, si::Length const observer_position_radius, si::Length const earth_radius, si::Length const atmosphere_radius)
{
	SlicesStacks result;

	// Longitude:
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
		auto const sun_vicinity = Range { sun_position.altitude - 20_deg, sun_position.altitude + 20_deg };

		// Start below the horizon:
		result.stack_angles.reserve (100);
		// Start at horizon (or a bit below):
		auto const horizon_angle = calculate_horizon_angle (earth_radius, observer_position_radius) - 0.5_deg;
		result.stack_angles.push_back (calculate_angle_from_origin_viewpoint (horizon_angle, atmosphere_radius, observer_position_radius));

		for (auto latitude = horizon_angle; latitude < 90_deg; )
		{
			result.stack_angles.push_back (calculate_angle_from_origin_viewpoint (latitude, atmosphere_radius, observer_position_radius));

			// TODO denser around the earth's visible border
			if (latitude <= 3_deg)
				latitude += 0.25_deg;
			else if (latitude <= 6_deg)
				latitude += 0.5_deg;
			else if (latitude <= 12_deg || sun_vicinity.includes (latitude))
				latitude += 1.5_deg;
			else
			{
				auto const big_step = 5_deg;

				if (latitude < sun_vicinity.min() && sun_vicinity.includes (latitude + big_step))
				{
					latitude = sun_vicinity.min() + 0.1_deg; // +0.1° prevents infinite loop.
					result.stack_angles.push_back (calculate_angle_from_origin_viewpoint (latitude, atmosphere_radius, observer_position_radius));
				}
				else
					latitude += big_step;
			}
		}

		// Top of the dome:
		result.stack_angles.push_back (90_deg);
	}

	return result;
}


[[nodiscard]]
SlicesStacks
calculate_ground_slices_and_stacks (si::Angle const horizon_angle, si::Length const earth_radius, si::Length const observer_position_radius)
{
	SlicesStacks result;

	// Longitude:
	{
		auto const n_slices = 50u;
		auto const delta = 360_deg / n_slices + 0.01_deg;
		result.slice_angles.reserve (360_deg / delta + 1);

		for (auto longitude = 0_deg; longitude < 360_deg; longitude += delta)
			result.slice_angles.push_back (longitude);

		// Complete the circle where we started:
		result.slice_angles.push_back (360_deg);
	}

	// Latitude:
	{
		auto const n_stacks = 50u;
		auto const visible_angle = 90_deg + horizon_angle;
		auto view_angle = calculate_angle_from_offset_viewpoint (visible_angle, earth_radius, observer_position_radius);
		auto const delta = (90_deg + view_angle) / n_stacks;

		result.stack_angles.reserve (n_stacks + 1);

		auto const dynamic_delta = [&view_angle, &delta]() {
			auto const factor = view_angle > -1_deg
				? 0.05
				: view_angle > -5_deg
					? 0.3
					: view_angle > -10_deg
						? 0.5
						: 1.0;
			return factor * delta;
		};

		// Denser near horizon at the expense of density directly under the camera:
		for (auto i = 0u; i < n_stacks; ++i, view_angle -= dynamic_delta())
		{
			auto const latitude = calculate_angle_from_origin_viewpoint (view_angle, earth_radius, observer_position_radius);
			result.stack_angles.push_back (latitude);
		}

		result.stack_angles.push_back (90_deg);
	}

	return result;
}


[[nodiscard]]
SlicesStacks
calculate_dome_slices_and_stacks (HorizontalCoordinates const sun_position, si::Angle const horizon_angle)
{
	SlicesStacks result;

	// Longitude:
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
		auto const n_ground_stacks = 50u;
		auto const horizon_epsilon = 0.001_deg;

		result.stack_angles.reserve (n_ground_stacks + 1 + 100);

		// Ground:
		{
			// TODO far fewer slices if really far away
			auto const delta = (horizon_angle + 90_deg) / n_ground_stacks;

			// Start from top, reverse later:
			auto latitude = horizon_angle - horizon_epsilon;

			// Denser near horizon at the expense of density directly under the camera:
			auto const dynamic_delta = [&latitude, &delta]() {
				auto const factor = latitude > -1_deg
					? 0.05
					: latitude > -5_deg
						? 0.3
						: latitude > -10_deg
							? 0.5
							: 1.0;
				return factor * delta;
			};

			for (auto i = 0u; i < n_ground_stacks; ++i, latitude -= dynamic_delta())
				result.stack_angles.push_back (latitude);

			// Final pole:
			result.stack_angles.push_back (-90_deg);

			// Latitudes must be in ascending order:
			std::ranges::reverse (result.stack_angles);
		}

		// FIXME there are rendering problems around the horizon_angle

		// Sky:
		{
			auto const sun_vicinity = Range { sun_position.altitude - 20_deg, sun_position.altitude + 20_deg };

			// Start at horizon:
			result.stack_angles.push_back (horizon_angle + horizon_epsilon);

			for (auto latitude = horizon_angle; latitude < 90_deg; )
			{
				result.stack_angles.push_back (latitude);

				// TODO denser around the earth's visible border when seen from space
				if (latitude <= horizon_angle + 3_deg)
					latitude += 0.25_deg;
				else if (latitude <= horizon_angle + 6_deg)
					latitude += 0.5_deg;
				else if (latitude <= horizon_angle + 12_deg || sun_vicinity.includes (latitude))
					latitude += 1.5_deg;
				else
				{
					auto const big_step = 5_deg;

					if (latitude < sun_vicinity.min() && sun_vicinity.includes (latitude + big_step))
					{
						latitude = sun_vicinity.min() + 0.001_deg; // +0.001° prevents infinite loop.
						result.stack_angles.push_back (latitude);
					}
					else
						latitude += big_step;
				}
			}

			// Top of the dome:
			result.stack_angles.push_back (90_deg);
		}
	}

	return result;
}


[[nodiscard]]
rigid_body::Shape
calculate_sky_shape (si::LonLatRadius const observer_position,
					 SkyDome::SunPosition const sun_position,
					 si::Length const earth_radius,
					 AtmosphericScattering const& atmospheric_scattering,
					 neutrino::WorkPerformer* work_performer = nullptr)
{
	auto const atmosphere_radius = atmospheric_scattering.parameters().atmosphere_radius;
	auto const ss = calculate_sky_slices_and_stacks (sun_position.horizontal_coordinates, observer_position.radius(), earth_radius, atmosphere_radius);
	// Sphere pole is at the top of the observer. Stacks are perpendicular to observer's local horizon:
	auto sky = rigid_body::make_centered_irregular_sphere_shape ({
		.radius = atmosphere_radius,
		.slice_angles = ss.slice_angles,
		.stack_angles = ss.stack_angles,
		.material = rigid_body::kTransparentBlack,
		.setup_material = [&] (rigid_body::ShapeMaterial& material, si::LonLat const sphere_position, WaitGroup::WorkToken&& work_token) {
			auto calculate = [&material, &atmospheric_scattering, &observer_position, &sun_position, atmosphere_radius, sphere_position, work_token = std::move (work_token)] {
				auto const view_angle = calculate_angle_from_offset_viewpoint (sphere_position.lat(), atmosphere_radius, observer_position.radius());
				auto const polar_ray_direction = si::LonLat (sphere_position.lon(), view_angle);
				auto const ray_direction = to_cartesian<void> (polar_ray_direction);
				auto const color = atmospheric_scattering.calculate_incident_light(
					{ 0_m, 0_m, observer_position.radius() },
					ray_direction,
					sun_position.cartesian_coordinates
				);
				material.gl_emission_color = to_gl_color (color);
			};

			if (work_performer)
				work_performer->submit (std::move (calculate));
			else
				calculate();
		},
	});
	rigid_body::negate_normals (sky);
	return sky;
}


[[nodiscard]]
rigid_body::Shape
calculate_ground_shape (si::LonLatRadius const observer_position,
						SkyDome::SunPosition const sun_position,
						si::Length const earth_radius,
						float const ground_haze_alpha,
						AtmosphericScattering const& atmospheric_scattering,
						neutrino::WorkPerformer* const work_performer = nullptr)
{
	auto const horizon_angle = calculate_horizon_angle (earth_radius, observer_position.radius());

	if (isfinite (horizon_angle))
	{
		auto const ss = calculate_ground_slices_and_stacks (horizon_angle, earth_radius, observer_position.radius());
		auto ground = rigid_body::make_centered_irregular_sphere_shape ({
			.radius = earth_radius,
			.slice_angles = ss.slice_angles,
			.stack_angles = ss.stack_angles,
			.material = rigid_body::kTransparentBlack,
			.setup_material = [&] (rigid_body::ShapeMaterial& material, si::LonLat const sphere_position, WaitGroup::WorkToken&& work_token) {
				auto calculate = [&material, &atmospheric_scattering, &observer_position, &sun_position, sphere_position, earth_radius, ground_haze_alpha, work_token = std::move (work_token)] {
					auto const cartesian_observer_position = SpaceLength (0_m, 0_m, observer_position.radius());
					auto const view_angle = calculate_angle_from_offset_viewpoint (sphere_position.lat(), earth_radius, observer_position.radius());
					auto const polar_ray_direction = si::LonLat (sphere_position.lon(), view_angle);
					auto const ray_direction = to_cartesian<void> (polar_ray_direction);
					si::Length max_distance = std::numeric_limits<si::Length>::infinity();

					if (auto const intersections = atmospheric_scattering.ray_sphere_intersections (cartesian_observer_position, ray_direction, earth_radius))
					{
						// If the ray intersects the Earth's sphere (planetary body) and the intersection is ahead of the camera:
						if (intersections->second > 0_m)
							// Limit the ray's effective length to the first intersection point.
							max_distance = std::max (0_m, intersections->first);
					}

					auto const color = atmospheric_scattering.calculate_incident_light(
						cartesian_observer_position,
						ray_direction,
						sun_position.cartesian_coordinates,
						0_m,
						max_distance
					);
					material.gl_emission_color = to_gl_color (color);
					material.gl_emission_color[3] = ground_haze_alpha;
				};

				if (work_performer)
					work_performer->submit (std::move (calculate));
				else
					calculate();
			},
		});
		return ground;
	}
	else
		return {};
}


[[nodiscard]]
rigid_body::Shape
calculate_dome_shape (si::LonLatRadius const observer_position,
					  SkyDome::SunPosition const sun_position,
					  si::Length const earth_radius,
					  float const ground_haze_alpha,
					  AtmosphericScattering const& atmospheric_scattering,
					  neutrino::WorkPerformer* const work_performer = nullptr)
{
	auto const horizon_angle = calculate_horizon_angle (earth_radius, observer_position.radius());

	// TODO still draw sky if horizon_angle is nan (assume it's 0° then)
	if (isfinite (horizon_angle))
	{
		auto const ss = calculate_dome_slices_and_stacks (sun_position.horizontal_coordinates, horizon_angle);
		auto ground = rigid_body::make_centered_irregular_sphere_shape ({
			// TODO 10 mm around the camera
			.radius = earth_radius,
			.slice_angles = ss.slice_angles,
			.stack_angles = ss.stack_angles,
			.material = rigid_body::kTransparentBlack,
			.setup_material = [&] (rigid_body::ShapeMaterial& material, si::LonLat const sphere_position, WaitGroup::WorkToken&& work_token) {
				auto calculate = [=, &material, &atmospheric_scattering, &observer_position, &sun_position, work_token = std::move (work_token)] {
					if (sphere_position.lat() > horizon_angle)
					{
						// Sky:
						auto const polar_ray_direction = si::LonLat (sphere_position.lon(), sphere_position.lat());
						auto const ray_direction = to_cartesian<void> (polar_ray_direction);
						auto const color = atmospheric_scattering.calculate_incident_light(
							{ 0_m, 0_m, observer_position.radius() },
							ray_direction,
							sun_position.cartesian_coordinates
						);
						material.gl_emission_color = to_gl_color (color);
					}
					else
					{
						// Ground haze:
						auto const cartesian_observer_position = SpaceLength (0_m, 0_m, observer_position.radius());
						auto const polar_ray_direction = si::LonLat (sphere_position.lon(), sphere_position.lat());
						auto const ray_direction = to_cartesian<void> (polar_ray_direction);
						si::Length max_distance = std::numeric_limits<si::Length>::infinity();

						if (auto const intersections = atmospheric_scattering.ray_sphere_intersections (cartesian_observer_position, ray_direction, earth_radius))
						{
							// If the ray intersects the Earth's sphere (planetary body) and the intersection is ahead of the camera:
							if (intersections->second > 0_m)
								// Limit the ray's effective length to the first intersection point.
								max_distance = std::max (0_m, intersections->first);
						}

						auto const color = atmospheric_scattering.calculate_incident_light(
							cartesian_observer_position,
							ray_direction,
							sun_position.cartesian_coordinates,
							0_m,
							max_distance
						);
						material.gl_emission_color = to_gl_color (color);
						material.gl_emission_color[3] = ground_haze_alpha;
					}
				};

				if (work_performer)
					work_performer->submit (std::move (calculate));
				else
					calculate();
			},
		});
		return ground;
	}
	else
		return {};
}


[[nodiscard]]
SpaceVector<float, RGBSpace>
calculate_sun_light_color (si::LonLatRadius const observer_position, SpaceVector<double> const sun_position, AtmosphericScattering const& atmospheric_scattering)
{
	return atmospheric_scattering.calculate_incident_light ({ 0_m, 0_m, observer_position.radius() }, sun_position, sun_position);
}


SkyDome
calculate_sky_dome (SkyDomeParameters const& params, neutrino::WorkPerformer* work_performer)
{
	auto const sun_position = calculate_sun_position (params.observer_position, params.unix_time);

	return {
		.sky_shape = calculate_sky_shape (params.observer_position, sun_position, params.earth_radius, params.atmospheric_scattering, work_performer),
		.ground_shape = calculate_ground_shape (params.observer_position, sun_position, params.earth_radius, params.ground_haze_alpha, params.atmospheric_scattering, work_performer),
		.dome_shape = calculate_dome_shape (params.observer_position, sun_position, params.earth_radius, params.ground_haze_alpha, params.atmospheric_scattering, work_performer),
		.sun_position = sun_position,
		.sun_light_color = calculate_sun_light_color (params.observer_position, sun_position.cartesian_coordinates, params.atmospheric_scattering),
	};
}

} // namespace xf

