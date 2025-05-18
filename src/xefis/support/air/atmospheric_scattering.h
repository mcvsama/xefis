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

#ifndef XEFIS__SUPPORT__AIR__ATMOSPHERIC_SCATTERING_H__INCLUDED
#define XEFIS__SUPPORT__AIR__ATMOSPHERIC_SCATTERING_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/color/spaces.h>
#include <xefis/support/math/geometry_types.h>
#include <xefis/support/nature/constants.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Implementation based on code from
 * <https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky.html>
 */
class AtmosphericScattering
{
  public:
	struct Parameters
	{
		// Radius of the ground:
		si::Length			earth_radius					{ kEarthMeanRadius };
		// Radius of the top of the sky:
		si::Length			atmosphere_radius				{ earth_radius + 60_km };

		// `rayleigh_threshold` and `mie_threshold` are characteristic distances over which the density falls by a factor of e, influencing the optical depth and the resulting light
		// attenuation in the sky dome model.

		// Thickness of the atmosphere if density was uniform; used for Rayleigh scattering:
		si::Length			rayleigh_threshold				{ 7994_m };
		// Thickness of the atmosphere if density was uniform; used for Mie scattering:
		si::Length			mie_threshold					{ 1200_m };
		// Output factor for light intensity from the the Rayleigh scattering:
		double				rayleigh_factor					{ 1.0 };
		// Output factor for light intensity from the the Mie scattering:
		double				mie_factor						{ 1.0 };
		// Automatically tonemap output values:
		bool				enable_tonemapping				{ false };
		// Number of samples taken along the view direction (casted ray):
		uint32_t			num_viewing_direction_samples	{ 64 };
		// Number of samples taken to the light source:
		uint32_t			num_light_direction_samples		{ 8 };
	};

	// Used for scaling output of AtmosphericScattering::calculate_incident_light(); chosen experimentally:
	static constexpr auto kIncidentLightScale = 100.0;

  public:
	// Ctor
	explicit
    AtmosphericScattering (Parameters const&);

	Parameters const&
	parameters() const noexcept
		{ return _p; }

	/**
	 * Calculate the light that reaches a `observer_position` point along a ray as it travels through the atmosphere, accounting for scattering effects.
	 * Kind of like ray-tracing of sky dome. Include both Rayleigh and Mie effects.
	 *
	 * \param	observer_position
	 *			Position of the observer.
	 * \param	ray_direction
	 *			This is the normalized direction vector of the ray; it indicates the path along which the light is integrated.
	 * \param	min_distance
	 *			The minimum distance along the ray from the observer_position where the integration (and thus the scattering calculation) begins. It can be
	 *			adjusted if the ray starts outside the atmosphere.
	 * \param	max_distance
	 *			The maximum distance along the ray where the calculation ends. It’s also adjusted based on where the ray exits the atmospheric sphere.
	 */
	[[nodiscard]]
    SpaceVector<float, RGBSpace>
	calculate_incident_light (
		SpaceLength<> const& observer_position,
		SpaceVector<double> const& ray_direction,
		SpaceVector<double> const& sun_direction,
		si::Length min_distance = 0_m,
		si::Length max_distance = std::numeric_limits<si::Length>::infinity()
	) const;

	[[nodiscard]]
	static constexpr float
	reinhard_tonemap (float value);

	[[nodiscard]]
	static constexpr float
	tonemap (float value);

	[[nodiscard]]
	static constexpr SpaceVector<float, RGBSpace>
	tonemap (SpaceVector<float, RGBSpace> input);

	[[nodiscard]]
	static constexpr SpaceVector<float, RGBSpace>
	tonemap_separately (SpaceVector<float, RGBSpace> input);

	/**
	 * Determine whether a ray intersects a sphere.
	 *
	 * This function tests if a ray—defined by its origin and normalized direction (direction)—intersects a sphere of a given radius (centered at the
	 * origin). If an intersection occurs, the distances along the ray to the intersection points are returned.
	 * Otherwise empty optional is returned.
	 *
	 * \param	ray_origin
	 *			The starting point of the ray (position of the observer).
	 * \param	ray_direction
	 *			The normalized direction vector of the ray.
	 * \param	radius The radius of the sphere.
	 * \param	optional<near, far>
	 *			Distances along the ray to the first (nearest) and the second intersection point.
	 */
	[[nodiscard]]
	static std::optional<std::pair<si::Length, si::Length>>
	ray_sphere_intersections (SpaceLength<> const& ray_origin, SpaceVector<double> const& ray_direction, si::Length const sphere_radius);

  private:
	Parameters			_p;
	decltype (1 / 1_m)	_inv_rayleigh_threshold				{ 1.0 / _p.rayleigh_threshold };
	decltype (1 / 1_m)	_inv_mie_threshold					{ 1.0 / _p.mie_threshold };
	double				_inv_num_light_direction_samples	{ 1.0 / _p.num_light_direction_samples };
};


constexpr float
AtmosphericScattering::reinhard_tonemap (float value)
{
	// The Reinhard operator compresses high dynamic range values
	// by mapping value to value / (1.0 + value), which approaches 1 as value increases:
	return value / (0.5 + value);
}


constexpr float
AtmosphericScattering::tonemap (float value)
{
	// If the channel's value is below a threshold (1.413), apply gamma correction;
	// otherwise, use an exponential curve to compress high values:
	//return reinhard_tonemap (value);
	return value < 1.413f
		? pow (value * 0.38317f, 1.0f / 2.2f)
		: 1.0f - exp (-value);
}


constexpr SpaceVector<float, RGBSpace>
AtmosphericScattering::tonemap (SpaceVector<float, RGBSpace> input)
{
	// Compute luminance using Rec.709 weights:
	auto const luminance = 0.2126f * input[0] + 0.7152f * input[1] + 0.0722f * input[2];
	auto const mapped_luminance = tonemap (luminance);

	// Avoid division by zero:
	if (luminance > 0.0f)
		return input * (mapped_luminance / luminance);
	else
		return input;
}


constexpr SpaceVector<float, RGBSpace>
AtmosphericScattering::tonemap_separately (SpaceVector<float, RGBSpace> input)
{
	// Apply tone mapping function to each color channel:
	for (auto& v: input.components())
		v = tonemap (v);

	return input;
}

} // namespace xf

#endif

