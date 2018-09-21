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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/logger.h>


namespace xf {

class Logger;


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
	Cycle (Number number, Time update_time, Time update_dt, Logger const&);

	/**
	 * Return this cycle serial number.
	 */
	[[nodiscard]]
	Number
	number() const noexcept;

	/**
	 * Return last update time.
	 */
	[[nodiscard]]
	si::Time
	update_time() const noexcept;

	/**
	 * Return time difference between last and previous update.
	 * Be sure not to use it if you're skipping some of the updates, because you're watching just one property or
	 * something.
	 */
	[[nodiscard]]
	si::Time
	update_dt() const noexcept;

	/**
	 * Return logger to use.
	 */
	[[nodiscard]]
	Logger const&
	logger() const noexcept;

  private:
	Number		_number;
	Time		_update_time;
	Time		_update_dt;
	Logger		_logger;
};


inline
Cycle::Cycle (Number number, Time update_time, Time update_dt, Logger const& logger):
	_number (number),
	_update_time (update_time),
	_update_dt (update_dt),
	_logger (logger)
{ }


inline Cycle::Number
Cycle::number() const noexcept
{
	return _number;
}


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


inline Logger const&
Cycle::logger() const noexcept
{
	return _logger;
}

} // namespace xf

#endif

