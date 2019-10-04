/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__EARTH__STANDARD_ATMOSPHERE_H__INCLUDED
#define XEFIS__SUPPORT__EARTH__STANDARD_ATMOSPHERE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/air/air.h>
#include <xefis/support/math/space.h>


namespace xf {

struct Air
{
	si::Density				density;
	si::Pressure			pressure;
	si::Temperature			temperature;
	si::DynamicViscosity	dynamic_viscosity;
	si::Velocity			speed_of_sound;
};


/**
 * StandardAtmosphere state at some given position.
 */
template<class Frame>
	struct AtmosphereState
	{
		Air									air;
		SpaceVector<si::Velocity, Frame>	wind;
	};


/**
 * General atmosphere model.
 * TODO use Perlin noise for winds. http://flafla2.github.io/2014/08/09/perlinnoise.html
 * TODO use winds to affect density/pressure at that point.
 * TODO maybe the winds model should be a separate class plugged in into Atmosphere object?
 */
class StandardAtmosphere
{
  public:
	[[nodiscard]]
	Air
	air_at (SpaceVector<si::Length, ECEFFrame> const& position) const;

	[[nodiscard]]
	Air
	air_at_radius (si::Length radius) const;

	[[nodiscard]]
	Air
	air_at_amsl (si::Length amsl_height) const;

	[[nodiscard]]
	SpaceVector<si::Velocity, ECEFFrame>
	wind_at (SpaceVector<si::Length, ECEFFrame> const& position) const;

	[[nodiscard]]
	AtmosphereState<ECEFFrame>
	state_at (SpaceVector<si::Length, ECEFFrame> const& position) const;
};


/*
 * Global functions
 */


si::Density
standard_density (si::Length geometric_altitude_amsl);

si::Pressure
standard_pressure (si::Length geometric_altitude_amsl);

si::Temperature
standard_temperature (si::Length geometric_altitude_amsl);

si::TemperatureGradient
standard_temperature_gradient (si::Length geometric_altitude_amsl);

si::DynamicViscosity
dynamic_air_viscosity (si::Temperature);

} // namespace xf

#endif

