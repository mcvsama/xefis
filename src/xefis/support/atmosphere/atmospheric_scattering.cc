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
#include <algorithm>
#include <cstddef>
#include <cmath>
#include <limits>
#include <numbers>


namespace xf {

AtmosphericScattering::AtmosphericScattering (Parameters const& parameters):
	_p (parameters)
{
	build_transmittance_lut();
}


void
AtmosphericScattering::build_transmittance_lut()
{
	_transmittance_lut_height_samples = std::max<std::size_t> (_p.transmittance_lut_height_samples, 2);
	_transmittance_lut_mu_samples = std::max<std::size_t> (_p.transmittance_lut_mu_samples, 2);

	auto const lut_size = _transmittance_lut_height_samples * _transmittance_lut_mu_samples;
	_transmittance_lut_rayleigh.assign (lut_size, 0_m);
	_transmittance_lut_mie.assign (lut_size, 0_m);
	_transmittance_lut_reaches_toa.assign (lut_size, 0);

	auto const atmosphere_height = _p.atmosphere_radius - _p.earth_radius;
	auto const num_light_samples = std::max (_p.num_light_direction_samples, 1u);
	auto const inv_num_light_samples = 1.0 / num_light_samples;

	for (std::size_t h = 0; h < _transmittance_lut_height_samples; ++h)
	{
		auto const height_alpha = static_cast<double> (h) / (_transmittance_lut_height_samples - 1);
		auto const height = height_alpha * atmosphere_height;
		auto const sample_position = SpaceLength<> { 0_m, 0_m, _p.earth_radius + height };

		for (std::size_t m = 0; m < _transmittance_lut_mu_samples; ++m)
		{
			auto const mu_alpha = static_cast<double> (m) / (_transmittance_lut_mu_samples - 1);
			auto const mu = -1.0 + 2.0 * mu_alpha;
			auto const sin_theta = std::sqrt (std::max (0.0, 1.0 - nu::square (mu)));
			auto const light_direction = SpaceVector<double> { sin_theta, 0.0, mu };
			auto const atmosphere_intersections = ray_sphere_intersections (sample_position, light_direction, _p.atmosphere_radius);

			if (!atmosphere_intersections || atmosphere_intersections->second <= 0_m)
				continue;

			auto const distance_to_atmosphere_boundary = atmosphere_intersections->second;
			auto reaches_top_of_atmosphere = true;

			if (auto const ground_intersections = ray_sphere_intersections (sample_position, light_direction, _p.earth_radius))
			{
				si::Length ground_distance = std::numeric_limits<si::Length>::infinity();

				if (ground_intersections->first > 0_m)
					ground_distance = ground_intersections->first;
				else if (ground_intersections->second > 0_m)
					ground_distance = ground_intersections->second;

				if (ground_distance < distance_to_atmosphere_boundary)
					reaches_top_of_atmosphere = false;
			}

			if (!reaches_top_of_atmosphere)
				continue;

			si::Length const light_segment_length = distance_to_atmosphere_boundary * inv_num_light_samples;
			auto light_current_distance = 0_m;
			auto light_optical_depth = RayleighMie { 0.0_m, 0.0_m };

			for (uint32_t i = 0; i < num_light_samples; ++i)
			{
				auto const light_sample_position = sample_position + (light_current_distance + light_segment_length * 0.5f) * light_direction;
				auto const light_height = light_sample_position.norm() - _p.earth_radius;

				if (light_height < 0_m)
				{
					reaches_top_of_atmosphere = false;
					break;
				}

				light_optical_depth.r += std::exp (-light_height * _inv_rayleigh_threshold) * light_segment_length;
				light_optical_depth.m += std::exp (-light_height * _inv_mie_threshold) * light_segment_length;
				light_current_distance += light_segment_length;
			}

			if (reaches_top_of_atmosphere)
			{
				auto const index = transmittance_lut_index (h, m);
				_transmittance_lut_rayleigh[index] = light_optical_depth.r;
				_transmittance_lut_mie[index] = light_optical_depth.m;
				_transmittance_lut_reaches_toa[index] = 1;
			}
		}
	}
}


std::optional<AtmosphericScattering::RayleighMie<si::Length>>
AtmosphericScattering::sample_transmittance_lut (si::Length const height, double const mu) const
{
	if (_transmittance_lut_height_samples < 2 || _transmittance_lut_mu_samples < 2)
		return std::nullopt;

	auto const atmosphere_height = _p.atmosphere_radius - _p.earth_radius;
	auto const atmosphere_height_m = atmosphere_height.in<si::Meter>();

	if (atmosphere_height_m <= 0.0)
		return std::nullopt;

	auto const h_alpha = std::clamp (height.in<si::Meter>() / atmosphere_height_m, 0.0, 1.0);
	auto const mu_alpha = std::clamp (0.5 * (mu + 1.0), 0.0, 1.0);
	auto const h_coord = h_alpha * (_transmittance_lut_height_samples - 1);
	auto const m_coord = mu_alpha * (_transmittance_lut_mu_samples - 1);
	auto const h0 = static_cast<std::size_t> (std::floor (h_coord));
	auto const m0 = static_cast<std::size_t> (std::floor (m_coord));
	auto const h1 = std::min (h0 + 1, _transmittance_lut_height_samples - 1);
	auto const m1 = std::min (m0 + 1, _transmittance_lut_mu_samples - 1);
	auto const h_t = h_coord - h0;
	auto const m_t = m_coord - m0;

	struct WeightedSample
	{
		std::size_t	h;
		std::size_t m;
		double		weight;
	};

	WeightedSample const samples[] = {
		{ h0, m0, (1.0 - h_t) * (1.0 - m_t) },
		{ h0, m1, (1.0 - h_t) * m_t },
		{ h1, m0, h_t * (1.0 - m_t) },
		{ h1, m1, h_t * m_t },
	};

	auto weight_sum = 0.0;
	auto rayleigh = 0.0_m;
	auto mie = 0.0_m;

	for (auto const& sample: samples)
	{
		auto const index = transmittance_lut_index (sample.h, sample.m);

		if (_transmittance_lut_reaches_toa[index] == 0)
			continue;

		weight_sum += sample.weight;
		rayleigh += _transmittance_lut_rayleigh[index] * sample.weight;
		mie += _transmittance_lut_mie[index] * sample.weight;
	}

	if (weight_sum <= 0.0)
		return std::nullopt;

	auto const inv_weight_sum = 1.0 / weight_sum;
	return RayleighMie { rayleigh * inv_weight_sum, mie * inv_weight_sum };
}


[[nodiscard]]
SpaceVector<float, RGBSpace>
AtmosphericScattering::compute_incident_light (SpaceLength<> const& observer_position,
											   SpaceVector<double> const& ray_direction,
											   SpaceVector<double> const& sun_direction,
											   si::Length min_distance,
											   si::Length max_distance) const
{
	// Precomputed values that correspond to the scattering coefficients of the sky at sea level, for wavelengths 680, 550 and 440 respectively:
	static constexpr SpaceVector<double, RGBSpace> kRayleighBeta	= { 5.802e-6f, 13.558e-6f, 33.1e-6f };
	// Mie scattering doesn't change the color, so the coefficients are the same:
	static constexpr SpaceVector<double, RGBSpace> kMieBeta			= { 21e-6f, 21e-6f, 21e-6f };

	auto intersections = ray_sphere_intersections (observer_position, ray_direction, _p.atmosphere_radius);

	if (!intersections || intersections->second < 0_m)
		return math::zero;

	auto const [near, far] = *intersections;

	if (far < 0_m)
		return math::zero;

	// Adjust min/max distance to ensure we are sampling only within the valid range:
	if (near > min_distance && near > 0_m)
		min_distance = near;

	if (far < max_distance)
		max_distance = far;

	// Compute the length of each sample segment along the view ray:
	si::Length sky_segment_length = (max_distance - min_distance) / _p.num_viewing_direction_samples;
	si::Length sky_current_distance = min_distance;

	// Compute phase functions (scattering intensity based on angle between sun and view direction):
	auto const g = 0.76; // Mie asymmetry factor (approximates forward scattering).
	auto const gg = nu::square (g);
	auto const mu = dot_product (ray_direction, sun_direction); // Cosine of the angle between the sun direction and the ray direction.
	auto const phase = RayleighMie<double> {
		.r = 3.0 / (16.0 * std::numbers::pi) * (1.0 + mu * mu),
		.m = 3.0 / (8.0 * std::numbers::pi) * ((1.0 - gg) * (1.0 + mu * mu)) / ((2.0 + gg) * std::pow (1.0 + gg - 2.0 * g * mu, 1.5)),
	};

	// Accumulate contributions from Rayleigh and Mie scattering:
	auto contribution = RayleighMie<SpaceVector<double, RGBSpace>> { math::zero, math::zero };
	auto sky_optical_depth = RayleighMie { 0.0_m, 0.0_m };

	// Take multiple samples from the observer_position to the upper limit of the atmosphere:
	for (uint32_t i = 0; i < _p.num_viewing_direction_samples; ++i)
	{
		// Compute the position of the current sample:
		auto const sky_sample_position = observer_position + (sky_current_distance + sky_segment_length * 0.5f) * ray_direction;
		auto const sky_sample_height = sky_sample_position.norm() - _p.earth_radius;

		if (sky_sample_height < 0_m)
		{
			sky_current_distance += sky_segment_length;
			continue;
		}

		// Compute optical depth for Rayleigh and Mie scattering:
		auto const hr = nu::fast_exp (-sky_sample_height * _inv_rayleigh_threshold) * sky_segment_length;
		auto const hm = nu::fast_exp (-sky_sample_height * _inv_mie_threshold) * sky_segment_length;
		sky_optical_depth.r += hr;
		sky_optical_depth.m += hm;

		auto const local_up = sky_sample_position / sky_sample_position.norm();
		auto const light_mu = dot_product (local_up, sun_direction);

		if (auto const light_optical_depth = sample_transmittance_lut (sky_sample_height, light_mu))
		{
			SpaceLength<RGBSpace> tau_rayleigh = kRayleighBeta * (sky_optical_depth.r + light_optical_depth->r);
			SpaceLength<RGBSpace> tau_mie = kMieBeta * 1.1f * (sky_optical_depth.m + light_optical_depth->m);
			SpaceLength<RGBSpace> tau = tau_rayleigh + tau_mie;
			SpaceVector<double, RGBSpace> const tau_float = tau / 1_m;
			SpaceVector<double, RGBSpace> const attenuation { nu::fast_exp (-tau_float[0]), nu::fast_exp (-tau_float[1]), nu::fast_exp (-tau_float[2]) };
			contribution.r += attenuation * hr.in<si::Meter>();
			contribution.m += attenuation * hm.in<si::Meter>();
		}

		sky_current_distance += sky_segment_length;
	}

	auto const rayleigh_result = _p.rayleigh_factor * hadamard_product (contribution.r, kRayleighBeta) * phase.r;
	auto const mie_result = _p.mie_factor * hadamard_product (contribution.m, kMieBeta) * phase.m;
	auto const color_double = kIncidentLightScale * (rayleigh_result + mie_result);
	auto color = math::static_components_cast<float> (color_double);

	if (_p.enable_tonemapping)
		color = tonemap_separately (color);

	// Change NaNs to 0 (in case we were trying to get color from some invalid out-of-atmosphere point):
	for (auto& component: color.components())
		if (!std::isfinite (component))
			component = 0.0f;

	return color;
}


std::optional<std::pair<si::Length, si::Length>>
AtmosphericScattering::ray_sphere_intersections (SpaceLength<> const& ray_origin, SpaceVector<double> const& ray_direction, si::Length const sphere_radius)
{
	auto const ray_origin_m = ray_origin / 1_m;
	auto const sphere_radius_m = sphere_radius.in<si::Meter>();
	auto const a = dot_product (ray_direction, ray_direction);
	auto const b = 2 * dot_product (ray_direction, ray_origin_m);
	auto const c = dot_product (ray_origin_m, ray_origin_m) - nu::square (sphere_radius_m);

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
