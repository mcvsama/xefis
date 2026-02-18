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

#ifndef XEFIS__SUPPORT__ATMOSPHERE__SIMULATED_ATMOSPHERE_H__INCLUDED
#define XEFIS__SUPPORT__ATMOSPHERE__SIMULATED_ATMOSPHERE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/atmosphere/air.h>
#include <xefis/support/atmosphere/atmosphere.h>
#include <xefis/support/atmosphere/standard_atmosphere.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Simulated atmosphere model. Similar to Standard atmosphere, but adds wind and some variations in other parameters as well.
 */
class SimulatedAtmosphere: public Atmosphere
{
  public:
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

  private:
	StandardAtmosphere _standard_atmosphere;
};

} // namespace xf

#endif
