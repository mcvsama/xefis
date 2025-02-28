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
#include "atmospheric_scattering.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/algorithms.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <cmath>
#include <numbers>


namespace xf {

/**
 * Helper struct used in AtmosphericScattering::calculate_incident_light().
 */
template<class Value>
	struct RayleighMie
	{
		Value r;
		Value m;
	};


AtmosphericScattering::AtmosphericScattering (Parameters const& parameters):
	_earth_radius (parameters.earth_radius),
	_atmosphere_radius (parameters.atmosphere_radius),
	_rayleigh_threshold (parameters.rayleigh_threshold),
	_mie_threshold (parameters.mie_threshold),
	_rayleigh_factor (parameters.rayleigh_factor),
	_mie_factor (parameters.mie_factor),
	_num_viewing_direction_samples (parameters.num_viewing_direction_samples),
	_num_light_direction_samples (parameters.num_light_direction_samples)
{ }


[[nodiscard]]
SpaceVector<float, RGBSpace>
AtmosphericScattering::calculate_incident_light (SpaceLength<> const& observer_position,
												 SpaceVector<double> const& ray_direction,
												 SpaceVector<double> const& sun_direction,
												 si::Length min_distance,
												 si::Length max_distance) const
{
	auto intersections = ray_sphere_intersections (observer_position, ray_direction, _atmosphere_radius);

	if (!intersections || intersections->second < 0_m)
		return math::zero;

	auto const [near, far] = *intersections;

	if (far < 0_m)
		return math::zero;
	else
	{
		if (near > min_distance && near > 0_m)
			min_distance = near;

		if (far < max_distance)
			max_distance = far;

		si::Length sky_segment_length = (max_distance - min_distance) / _num_viewing_direction_samples;
		si::Length sky_current_distance = min_distance;

		auto const g = 0.76;
		auto const gg = neutrino::square (g);
		auto const mu = dot_product (ray_direction, sun_direction); // Cosine of the angle between the sun direction and the ray direction.
		auto const phase = RayleighMie<double> {
			.r = 3.0 / (16.0 * std::numbers::pi) * (1.0 + mu * mu),
			.m = 3.0 / (8.0 * std::numbers::pi) * ((1.0 - gg) * (1.0 + mu * mu)) / ((2.0 + gg) * std::pow (1.0 + gg - 2.0 * g * mu, 1.5)),
		};
		auto contribution = RayleighMie<SpaceVector<double>> { math::zero, math::zero };
		auto sky_optical_depth = RayleighMie { 0.0_m, 0.0_m };

		// Take multiple samples from the observer_position to the upper limit of the atmosphere:
		for (uint32_t i = 0; i < _num_viewing_direction_samples; ++i)
		{
			auto const sky_sample_position = observer_position + (sky_current_distance + sky_segment_length * 0.5f) * ray_direction;
			auto const light_intersections = ray_sphere_intersections (sky_sample_position, sun_direction, _atmosphere_radius);

			if (light_intersections)
			{
				auto const sky_sample_height = sky_sample_position.norm() - _earth_radius;
				auto const hr = std::exp (-sky_sample_height / _rayleigh_threshold) * sky_segment_length;
				auto const hm = std::exp (-sky_sample_height / _mie_threshold) * sky_segment_length;
				sky_optical_depth.r += hr;
				sky_optical_depth.m += hm;

				auto const [light_near_intersection, light_far_intersection] = *light_intersections;
				si::Length const light_segment_length = light_far_intersection / _num_light_direction_samples;
				auto light_current_distance = 0_m;
				auto light_optical_depth = RayleighMie { 0.0_m, 0.0_m };
				auto light_samples_taken = 0u;

				// At each atmospheric sampling point, calculate light reflected towards the observer by going in the direction
				// of light source and taking multiple samples until the top of the atmosphere:
				for (; light_samples_taken < _num_light_direction_samples; ++light_samples_taken)
				{
					auto const light_sample_position = sky_sample_position + (light_current_distance + light_segment_length * 0.5f) * sun_direction;
					auto const light_height = light_sample_position.norm() - _earth_radius;

					if (light_height < 0_m)
						break;

					light_optical_depth.r += std::exp (-light_height / _rayleigh_threshold) * light_segment_length;
					light_optical_depth.m += std::exp (-light_height / _mie_threshold) * light_segment_length;
					light_current_distance += light_segment_length;
				}

				if (light_samples_taken == _num_light_direction_samples)
				{
					SpaceVector<si::Length> tau = kRayleighBeta * (sky_optical_depth.r + light_optical_depth.r) + kMieBeta * 1.1f * (sky_optical_depth.m + light_optical_depth.m);
					SpaceVector<double> const tau_float = tau / 1_m;
					SpaceVector<double> const attenuation { exp (-tau_float[0]), exp (-tau_float[1]), exp (-tau_float[2]) };
					contribution.r += attenuation * hr / 1_m;
					contribution.m += attenuation * hm / 1_m;
				}

				sky_current_distance += sky_segment_length;
			}
		}

		auto const rayleigh_result = _rayleigh_factor * hadamard_product (contribution.r, kRayleighBeta) * phase.r;
		auto const mie_result = _mie_factor * hadamard_product (contribution.m, kMieBeta) * phase.m;
		auto const result = kIncidentLightScale * (rayleigh_result + mie_result);

		return SpaceVector<float, RGBSpace> { result[0], result[1], result[2] };
	}
}


std::optional<std::pair<si::Length, si::Length>>
AtmosphericScattering::ray_sphere_intersections (SpaceLength<> const& ray_origin, SpaceVector<double> const& ray_direction, si::Length const sphere_radius)
{
	auto const ray_origin_m = ray_origin / 1_m;
	auto const sphere_radius_m = sphere_radius / 1_m;
	auto const a = dot_product (ray_direction, ray_direction);
	auto const b = 2 * dot_product (ray_direction, ray_origin_m);
	auto const c = dot_product (ray_origin_m, ray_origin_m) - square (sphere_radius_m);

	if (auto solution = solve_quadratic (a, b, c))
	{
		if (solution->first > solution->second)
			std::swap (solution->first, solution->second);

		return std::make_optional<std::pair<si::Length, si::Length>> (1_m * solution->first, 1_m * solution->second);
	}
	else
		return std::nullopt;
}

} // namespace xf

