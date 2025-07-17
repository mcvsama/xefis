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

#ifndef XEFIS__CORE__CLOCK_H__INCLUDED
#define XEFIS__CORE__CLOCK_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/time.h>

// Standard:
#include <cstddef>
#include <memory>


namespace xf {

class Clock
{
  public:
	// Dtor
	virtual
	~Clock() = default;

	[[nodiscard]]
	virtual si::Time
	now() const = 0;
};


class UTCClock: public Clock
{
  public:
	// Ctor
	explicit
	UTCClock()
		{ set (nu::utc_now()); }

	void
	set (si::Time const unix_time)
		{ _delta = unix_time - nu::steady_now(); }

	// Clock API
	[[nodiscard]]
	si::Time
	now() const override
		{ return nu::steady_now() + _delta; }

  private:
	si::Time _delta;
};


class SteadyClock: public Clock
{
  public:
	// Clock API
	[[nodiscard]]
	si::Time
	now() const override
		{ return nu::steady_now(); }
};


class SimulatedClock: public Clock
{
  public:
	// Ctor
	explicit
	SimulatedClock(si::Time const initial_time):
		_now (initial_time)
	{ }

	void
	set (si::Time const unix_time)
		{ _now = unix_time; }

	void
	advance (si::Time const duration)
		{ _now += duration; }

	// Clock API
	[[nodiscard]]
	si::Time
	now() const override
		{ return _now; }

  private:
	si::Time _now;
};

} // namespace xf

#endif

