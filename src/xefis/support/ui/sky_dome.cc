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
	auto const cartesian_coordinates = cartesian<void> (si::LonLat (-horizontal_coordinates.azimuth + 180_deg, horizontal_coordinates.altitude));

	return {
		.hour_angle = hour_angle,
		.declination = declination,
		.horizontal_coordinates = horizontal_coordinates,
		.cartesian_coordinates = cartesian_coordinates,
	};
}


[[nodiscard]]
SlicesStacks
calculate_sky_slices_and_stacks (HorizontalCoordinates const sun_position)
{
	SlicesStacks result;

	// Longitude:
	{
		result.slice_angles.clear();

		auto constexpr sun_vicinity_slices = 13;
		auto constexpr rest_slices = 30;

		auto const sun_longitude = 180_deg - sun_position.azimuth;
		auto const sun_vicinity = Range { sun_longitude - 20_deg, sun_longitude + 20_deg };
		auto const sun_vicinity_delta = sun_vicinity.extent() / (sun_vicinity_slices - 1);
		auto const rest_range = 360_deg - sun_vicinity.extent();
		auto const rest_delta = rest_range / (rest_slices - 1);

		auto longitude = sun_vicinity.min();

		for (auto i = 0u; i < sun_vicinity_slices; ++i, longitude += sun_vicinity_delta)
			result.slice_angles.push_back (longitude);

		for (auto i = 0u; i < rest_slices; ++i, longitude += rest_delta)
			result.slice_angles.push_back (longitude);
	}

	// Latitude:
	{
		auto const sun_vicinity = Range { sun_position.altitude - 10_deg, sun_position.altitude + 10_deg };

		// Start below the horizon:
		result.stack_angles = { -10_deg };

		for (auto latitude = 0_deg; latitude < 90_deg; )
		{
			result.stack_angles.push_back (latitude);

			if (latitude <= 3_deg)
				latitude += 0.25_deg;
			else if (latitude <= 6_deg)
				latitude += 0.5_deg;
			else if (sun_vicinity.includes (latitude))
				latitude += 1_deg;
			else
			{
				auto const big_step = 5_deg;

				if (latitude < sun_vicinity.min() && sun_vicinity.includes (latitude + big_step))
				{
					latitude = sun_vicinity.min() + 0.1_deg; // +0.1° prevents infinite loop.
					result.stack_angles.push_back (latitude);
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
rigid_body::Shape
calculate_sky_shape (xf::LonLatRadius const observer_position,
					 SkyDome::SunPosition const sun_position,
					 si::Length const horizon_radius,
					 AtmosphericScattering const& atmospheric_scattering,
					 neutrino::WorkPerformer* work_performer = nullptr)
{
	auto const ss = calculate_sky_slices_and_stacks (sun_position.horizontal_coordinates);
	// Sphere pole is at the top of the observer. Stacks are perpendicular to observer's local horizon:
	auto sky = rigid_body::make_centered_irregular_sphere_shape ({
		.radius = horizon_radius,
		.slice_angles = ss.slice_angles,
		.stack_angles = ss.stack_angles,
		.material = rigid_body::kBlackMatte,
		.setup_material = [&] (rigid_body::ShapeMaterial& material, si::LonLat const sphere_position, WaitGroup::WorkToken&& work_token) {
			auto calculate = [&material, &atmospheric_scattering, &observer_position, &sun_position, sphere_position, work_token = std::move (work_token)] {
				auto const color = atmospheric_scattering.calculate_incident_light (
					{ 0_m, 0_m, observer_position.radius() },
					cartesian<void> (sphere_position),
					sun_position.cartesian_coordinates
				);
				auto const tonemapped_color = color;
				material.gl_emission_color = to_gl_color (tonemapped_color);
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
calculate_ground_shape()
{
	// TODO radius equal to AGL height
	// TODO the shape should be blended like this with the real ground:
	// glEnable(GL_BLEND);
	// glBlendFunc(GL_ONE, GL_ONE);
	return {};
}


[[nodiscard]]
SpaceVector<float, RGBSpace>
calculate_sun_light_color (xf::LonLatRadius const observer_position, SpaceVector<double> const sun_position, AtmosphericScattering const& atmospheric_scattering)
{
	return atmospheric_scattering.calculate_incident_light ({ 0_m, 0_m, observer_position.radius() }, sun_position, sun_position);
}


SkyDome
calculate_sky_dome (SkyDomeParameters const& params, neutrino::WorkPerformer* work_performer)
{
	auto const sun_position = calculate_sun_position (params.observer_position, params.unix_time);

	return {
		.sky_shape = calculate_sky_shape (params.observer_position, sun_position, params.horizon_radius, params.atmospheric_scattering, work_performer),
		.ground_shape = calculate_ground_shape(),
		.sun_position = sun_position,
		.sun_light_color = calculate_sun_light_color (params.observer_position, sun_position.cartesian_coordinates, params.atmospheric_scattering),
	};
}

} // namespace xf

