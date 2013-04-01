/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__UTILITY__DENSITY_ALTITUDE_H__INCLUDED
#define XEFIS__UTILITY__DENSITY_ALTITUDE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>


namespace Xefis {

class DensityAltitude
{
  public:
	/**
	 * Set pressure altitude.
	 */
	void
	set_pressure_altitude (Length);

	/**
	 * Set actual temperature.
	 */
	void
	set_outside_air_temperature (float kelvins);

	/**
	 * Calculate result.
	 */
	void
	update();

	/**
	 * Return resulting DA.
	 */
	Length
	density_altitude() const;

  private:
	Length	_pressure_altitude;
	float	_outside_air_temperature_k;
	Length	_density_altitude;
};


inline void
DensityAltitude::set_pressure_altitude (Length altitude)
{
	_pressure_altitude = altitude;
}


inline void
DensityAltitude::set_outside_air_temperature (float kelvins)
{
	_outside_air_temperature_k = kelvins;
}


inline void
DensityAltitude::update()
{
	float t_s = 273.15 + (15.0 - (0.0019812 * _pressure_altitude.ft()));
	_density_altitude = _pressure_altitude + 1_ft * (t_s / 0.0019812) * (1.0 - std::pow (t_s / _outside_air_temperature_k, 0.2349690));
}


inline Length
DensityAltitude::density_altitude() const
{
	return _density_altitude;
}

} // namespace Xefis

#endif

