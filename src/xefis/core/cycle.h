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

#ifndef XEFIS__CORE__CYCLE_H__INCLUDED
#define XEFIS__CORE__CYCLE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/logger.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Holds useful information about the single processing cycle.
 */
class Cycle
{
  public:
	using Number = uint64_t;

  public:
	// Ctor
	explicit
	Cycle (Number number, si::Time update_time, si::Time update_dt, si::Time intended_update_dt, Logger const&);

	/**
	 * Return this cycle serial number.
	 */
	[[nodiscard]]
	Number
	number() const noexcept
		{ return _number; }

	/**
	 * Return last update time.
	 */
	[[nodiscard]]
	si::Time
	update_time() const noexcept
		{ return _update_time; }

	/**
	 * Return time difference between last and previous update.
	 * Be sure not to use it if you're skipping some of the updates,
	 * because you're watching just one socket or something.
	 */
	[[nodiscard]]
	si::Time
	update_dt() const noexcept
		{ return _update_dt; }

	/**
	 * Return intended time difference between last and previous update as configured in the ProcessingLoop.
	 */
	[[nodiscard]]
	si::Time
	intended_update_dt() const noexcept
		{ return _intended_update_dt; }

	/**
	 * Return logger to use.
	 */
	[[nodiscard]]
	Logger const&
	logger() const noexcept
		{ return _logger; }

  private:
	Number		_number;
	si::Time	_update_time;
	si::Time	_update_dt;
	si::Time	_intended_update_dt;
	Logger		_logger;
};


inline
Cycle::Cycle (Number number, si::Time update_time, si::Time update_dt, si::Time intended_update_dt, Logger const& logger):
	_number (number),
	_update_time (update_time),
	_update_dt (update_dt),
	_intended_update_dt (intended_update_dt),
	_logger (logger)
{ }

} // namespace xf

#endif

