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

#ifndef XEFIS__UTILITY__SOUND_SPEED_H__INCLUDED
#define XEFIS__UTILITY__SOUND_SPEED_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>


namespace Xefis {

class SoundSpeed
{
  public:
	/**
	 * Set static air temperature.
	 */
	void
	set_static_air_temperature (Temperature temperature);

	/**
	 * Calculate result.
	 */
	void
	update();

	/**
	 * Return resulting speed of sound.
	 */
	Speed
	sound_speed() const;

  private:
	Temperature	_static_air_temperature;
	Speed		_sound_speed;
};


inline void
SoundSpeed::set_static_air_temperature (Temperature temperature)
{
	_static_air_temperature = temperature;
}


inline void
SoundSpeed::update()
{
	_sound_speed = 38.967854_kt * std::sqrt (_static_air_temperature.K());
}


inline Speed
SoundSpeed::sound_speed() const
{
	return _sound_speed;
}

} // namespace Xefis

#endif

