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

#ifndef XEFIS__SUPPORT__EARTH__ATMOSPHERE_MODEL_H__INCLUDED
#define XEFIS__SUPPORT__EARTH__ATMOSPHERE_MODEL_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/air/air.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>


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
template<class Space>
	struct AtmosphereState
	{
		Air									air;
		SpaceVector<si::Velocity, Space>	wind;
	};


/**
 * General atmosphere model.
 */
class AtmosphereModel
{
  public:
	// Dtor
	virtual
	~AtmosphereModel() = default;

	[[nodiscard]]
	virtual Air
	air_at (SpaceVector<si::Length, ECEFSpace> const& position) const = 0;

	[[nodiscard]]
	virtual Air
	air_at_radius (si::Length radius) const = 0;

	[[nodiscard]]
	virtual Air
	air_at_amsl (si::Length amsl_height) const = 0;

	[[nodiscard]]
	virtual SpaceVector<si::Velocity, ECEFSpace>
	wind_at (SpaceVector<si::Length, ECEFSpace> const& position) const = 0;

	[[nodiscard]]
	virtual AtmosphereState<ECEFSpace>
	state_at (SpaceVector<si::Length, ECEFSpace> const& position) const = 0;
};

} // namespace xf

#endif

