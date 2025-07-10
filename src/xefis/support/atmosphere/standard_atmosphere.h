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

#ifndef XEFIS__SUPPORT__ATMOSPHERE__STANDARD_ATMOSPHERE_H__INCLUDED
#define XEFIS__SUPPORT__ATMOSPHERE__STANDARD_ATMOSPHERE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/atmosphere/air.h>
#include <xefis/support/atmosphere/atmosphere.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Standard atmosphere model.
 */
class StandardAtmosphere: public Atmosphere
{
  public:
	// Atmosphere API
	[[nodiscard]]
	Air<ECEFSpace>
	air_at (SpaceLength<ECEFSpace> const& position) const override;

	[[nodiscard]]
	Air<ECEFSpace>
	air_at_radius (si::Length radius) const;

	[[nodiscard]]
	Air<ECEFSpace>
	air_at_amsl (si::Length amsl_height) const;

	// Atmosphere API
	[[nodiscard]]
	si::Density
	density_at (SpaceLength<ECEFSpace> const& position) const override;

	// Atmosphere API
	[[nodiscard]]
	si::Pressure
	pressure_at (SpaceLength<ECEFSpace> const& position) const override;

	// Atmosphere API
	[[nodiscard]]
	si::Temperature
	temperature_at (SpaceLength<ECEFSpace> const& position) const override;

	// Atmosphere API
	[[nodiscard]]
	SpaceVector<si::Velocity, ECEFSpace>
	wind_velocity_at (SpaceLength<ECEFSpace> const& position) const override;
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

} // namespace xf

#endif

