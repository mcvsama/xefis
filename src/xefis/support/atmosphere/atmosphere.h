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

#ifndef XEFIS__SUPPORT__ATMOSPHERE__ATMOSPHERE_H__INCLUDED
#define XEFIS__SUPPORT__ATMOSPHERE__ATMOSPHERE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/atmosphere/air.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * General atmosphere model.
 */
class Atmosphere
{
  public:
	// Dtor
	virtual
	~Atmosphere() = default;

	/**
	 * Gather air information for given position.
	 * Default implementation uses density_at(), pressure_at(), etc.
	 * to gather the data.
	 */
	[[nodiscard]]
	virtual Air<ECEFSpace>
	air_at (SpaceLength<ECEFSpace> const& position) const;

	[[nodiscard]]
	virtual si::Density
	density_at (SpaceLength<ECEFSpace> const& position) const = 0;

	[[nodiscard]]
	virtual si::Pressure
	pressure_at (SpaceLength<ECEFSpace> const& position) const = 0;

	[[nodiscard]]
	virtual si::Temperature
	temperature_at (SpaceLength<ECEFSpace> const& position) const = 0;

	[[nodiscard]]
	virtual SpaceVector<si::Velocity, ECEFSpace>
	wind_velocity_at (SpaceLength<ECEFSpace> const& position) const = 0;
};

} // namespace xf

#endif

