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

#ifndef XEFIS__UTILITY__WIND_TRIANGLE_H__INCLUDED
#define XEFIS__UTILITY__WIND_TRIANGLE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>


namespace Xefis {

class WindTriangle
{
  public:
	/**
	 * Set aircraft's true airspeed.
	 */
	void
	set_aircraft_tas (Speed);

	/**
	 * Set aircraft's track (course).
	 */
	void
	set_aircraft_track (Angle);

	/**
	 * Set aircraft's ground speed.
	 */
	void
	set_aircraft_ground_speed (Speed);

	/**
	 * Set aircraft's heading.
	 */
	void
	set_aircraft_heading (Angle);

	/**
	 * Calculate result.
	 */
	void
	update();

	/**
	 * Return resulting wind speed.
	 */
	Speed
	wind_speed() const;

	/**
	 * Return resulting wind direction
	 * (heading FROM which wind blows).
	 */
	Angle
	wind_direction() const;

  private:
	Speed	_a_tas;
	Angle	_a_track;
	Speed	_a_gs;
	Angle	_a_heading;
	Speed	_w_speed;
	Angle	_w_direction;
};


inline void
WindTriangle::set_aircraft_tas (Speed tas)
{
	_a_tas = tas;
}


inline void
WindTriangle::set_aircraft_track (Angle track)
{
	_a_track = track;
}


inline void
WindTriangle::set_aircraft_ground_speed (Speed gs)
{
	_a_gs = gs;
}


inline void
WindTriangle::set_aircraft_heading (Angle heading)
{
	_a_heading = heading;
}


inline void
WindTriangle::update()
{
	_w_speed = 1.0_kt * std::pow ((_a_tas - _a_gs).kt(), 2.0)
			 + 4.0_kt * _a_tas.kt() * _a_gs.kt() * std::pow (std::sin ((_a_heading - _a_track) / 2.0), 2.0);
	_w_speed = 1_kt * std::sqrt (_w_speed.kt());

	_w_direction = 1_rad * (_a_track.rad() + std::atan2 ((_a_tas * std::sin (_a_heading - _a_track)).kt(),
														 (_a_tas * std::cos (_a_heading - _a_track) - _a_gs).kt()));
	_w_direction = floored_mod (_w_direction, 360_deg);
}


inline Speed
WindTriangle::wind_speed() const
{
	return _w_speed;
}


inline Angle
WindTriangle::wind_direction() const
{
	return _w_direction;
}

} // namespace Xefis

#endif

