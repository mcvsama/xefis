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

#ifndef XEFIS__SUPPORT__EARTH__NAVIGATION__WIND_TRIANGLE_H__INCLUDED
#define XEFIS__SUPPORT__EARTH__NAVIGATION__WIND_TRIANGLE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <complex>


namespace xf {

/**
 * Note - headings should be all magnetic or all true.
 * Result will be magnetic or true, depending on what you give on input.
 */
class WindTriangle
{
  public:
	/**
	 * Set aircraft TAS (true air speed) and heading.
	 * The angle doesn't have to be normalized to 0..360°.
	 */
	void
	set_air_vector (si::Speed true_air_speed, si::Angle heading);

	/**
	 * Set ground speed and ground track.
	 * The angle doesn't have to be normalized to 0..360°.
	 */
	void
	set_ground_vector (si::Speed ground_speed, si::Angle track);

	/**
	 * Set wind speed and direction.
	 * The angle doesn't have to be normalized to 0..360°.
	 *
	 * NOTE: This is the direction the wind blows to, not from.
	 * Add 180° to the angle if you have a 'from' angle.
	 */
	void
	set_wind_vector (si::Speed wind_speed, si::Angle direction);

	/**
	 * Compute air vector.
	 */
	void
	compute_air_vector();

	/**
	 * Compute ground vector.
	 */
	void
	compute_ground_vector();

	/**
	 * Compute wind vector.
	 */
	void
	compute_wind_vector();

	/**
	 * Return resulting air speed (TAS).
	 */
	si::Speed
	air_speed() const;

	/**
	 * Return resulting air direction (aircraft heading).
	 * Result is normalized to 0..360°.
	 */
	si::Angle
	air_direction() const;

	/**
	 * Return resulting ground speed.
	 */
	si::Speed
	ground_speed() const;

	/**
	 * Return resulting ground direction (ground track).
	 * Result is normalized to 0..360°.
	 */
	si::Angle
	ground_direction() const;

	/**
	 * Return resulting wind speed.
	 */
	si::Speed
	wind_speed() const;

	/**
	 * Return resulting wind direction (heading TO which wind blows).
	 * Result is normalized to 0..360°.
	 */
	si::Angle
	wind_direction() const;

	/**
	 * Return resulting wind direction (heading FROM which wind blows).
	 * Result is normalized to 0..360°.
	 */
	si::Angle
	wind_from() const;

	/**
	 * Compute ground speed in given direction.
	 */
	si::Speed
	get_ground_speed (si::Angle heading) const;

  private:
	// Speeds are in mps() and angles in rad():
	std::complex<double>	_air_vector;
	std::complex<double>	_ground_vector;
	std::complex<double>	_wind_vector;
};


inline void
WindTriangle::set_air_vector (si::Speed true_air_speed, si::Angle heading)
{
	_air_vector = std::polar (true_air_speed.in<si::MeterPerSecond>(), heading.in<si::Radian>());
}


inline void
WindTriangle::set_ground_vector (si::Speed ground_speed, si::Angle track)
{
	_ground_vector = std::polar (ground_speed.in<si::MeterPerSecond>(), track.in<si::Radian>());
}


inline void
WindTriangle::set_wind_vector (si::Speed wind_speed, si::Angle direction)
{
	_wind_vector = std::polar (wind_speed.in<si::MeterPerSecond>(), direction.in<si::Radian>());
}


inline void
WindTriangle::compute_wind_vector()
{
	// G = A + W;  W = G - A
	_wind_vector = _ground_vector - _air_vector;
}


inline void
WindTriangle::compute_air_vector()
{
	// G = A + W;  A = G - W
	_air_vector = _ground_vector - _wind_vector;
}


inline void
WindTriangle::compute_ground_vector()
{
	// G = A + W
	_ground_vector = _air_vector + _wind_vector;
}


inline si::Speed
WindTriangle::air_speed() const
{
	return 1_mps * std::abs (_air_vector);
}


inline si::Angle
WindTriangle::air_direction() const
{
	return floored_mod (1_rad * std::arg (_air_vector), 360_deg);
}


inline si::Speed
WindTriangle::ground_speed() const
{
	return 1_mps * std::abs (_ground_vector);
}


inline si::Angle
WindTriangle::ground_direction() const
{
	return floored_mod (1_rad * std::arg (_ground_vector), 360_deg);
}


inline si::Speed
WindTriangle::wind_speed() const
{
	return 1_mps * std::abs (_wind_vector);
}


inline si::Angle
WindTriangle::wind_direction() const
{
	return floored_mod (1_rad * std::arg (_wind_vector), 360_deg);
}


inline si::Angle
WindTriangle::wind_from() const
{
	return floored_mod (wind_direction() + 180_deg, 360_deg);
}


inline si::Speed
WindTriangle::get_ground_speed (si::Angle aircraft_heading) const
{
	WindTriangle wt (*this);
	wt.set_air_vector (air_speed(), aircraft_heading);
	wt.compute_ground_vector();
	return wt.ground_speed();
}

} // namespace xf

#endif

