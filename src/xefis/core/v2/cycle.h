/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__V2__CYCLE_H__INCLUDED
#define XEFIS__CORE__V2__CYCLE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace x2 {

/**
 * Holds useful information about the single processing cycle.
 */
class Cycle
{
  public:
	// Ctor
	Cycle (Time update_time, Time update_dt);

	/**
	 * TODO
	 * Return last update time.
	 */
	Time
	update_time() const noexcept;

	/**
	 * TODO
	 * Return time difference between last and previous update.
	 * Be sure not to use it if you're skipping some of the updates, because you're watching just one property or
	 * something.
	 */
	Time
	update_dt() const noexcept;

  private:
	Time	_update_time;
	Time	_update_dt;
};


inline
Cycle::Cycle (Time update_time, Time update_dt):
	_update_time (update_time),
	_update_dt (update_dt)
{ }


inline Time
Cycle::update_time() const noexcept
{
	return _update_time;
}


inline Time
Cycle::update_dt() const noexcept
{
	return _update_dt;
}

} // namespace x2

#endif

