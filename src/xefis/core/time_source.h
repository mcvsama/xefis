/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__TIME_SOURCE_H__INCLUDED
#define XEFIS__CORE__TIME_SOURCE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/time_helper.h>

// Standard:
#include <cstddef>


namespace xf {

class TimeSource
{
  public:
	// Dtor
	virtual
	~TimeSource() = default;

	virtual si::Time
	now() = 0;
};


class UTCTimeSource: public TimeSource
{
  public:
	// TimeSource API
	si::Time
	now() override
		{ return nu::TimeHelper::utc_now(); }
};


class SimulatedTimeSource: public TimeSource
{
  public:
	// Ctor
	explicit
	SimulatedTimeSource (si::Time initial_time);

	void
	advance (si::Time const duration)
		{ _now += duration; }

	// TimeSource API
	si::Time
	now() override
		{ return _now; }

  private:
	si::Time _now;
};


inline
SimulatedTimeSource::SimulatedTimeSource (si::Time const initial_time):
	_now (initial_time)
{ }

} // namespace xf

#endif

