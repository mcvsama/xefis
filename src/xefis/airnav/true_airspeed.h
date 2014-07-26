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

#ifndef XEFIS__UTILITY__TRUE_AIRSPEED_H__INCLUDED
#define XEFIS__UTILITY__TRUE_AIRSPEED_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>


namespace Xefis {

/**
 * This does not take into account air compressibility factor, so it's valid
 * for low speeds (mach < 0.3) and altitude below tropopause (36 kft).
 */
class TrueAirspeed
{
  public:
	/**
	 * Set density altitude.
	 */
	void
	set_density_altitude (Length);

	/**
	 * Set indicated airspeed.
	 */
	void
	set_ias (Speed);

	/**
	 * Set true airspeed.
	 */
	void
	set_tas (Speed);

	/**
	 * Compute IAS from DA and TAS.
	 */
	void
	compute_ias();

	/**
	 * Compute TAS from DA and IAS.
	 */
	void
	compute_tas();

	/**
	 * Get set or computed IAS.
	 */
	Speed
	ias() const noexcept;

	/**
	 * Get set or computed TAS.
	 */
	Speed
	tas() const noexcept;

  private:
	Length		_density_altitude;
	Speed		_ias;
	Speed		_tas;
};


inline void
TrueAirspeed::set_density_altitude (Length altitude)
{
	_density_altitude = altitude;
}


inline void
TrueAirspeed::set_ias (Speed ias)
{
	_ias = ias;
}


inline void
TrueAirspeed::set_tas (Speed tas)
{
	_tas = tas;
}


inline void
TrueAirspeed::compute_ias()
{
	_ias = _tas * std::pow (1.0 - 6.8755856 * 1e-6 * _density_altitude.ft(), 2.127940);
}


inline void
TrueAirspeed::compute_tas()
{
	_tas = _ias / std::pow (1.0 - 6.8755856 * 1e-6 * _density_altitude.ft(), 2.127940);
}


inline Speed
TrueAirspeed::ias() const noexcept
{
	return _ias;
}


inline Speed
TrueAirspeed::tas() const noexcept
{
	return _tas;
}

} // namespace Xefis

#endif

