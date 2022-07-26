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

#ifndef XEFIS__UTILITY__EVENT_TIMESTAMPER_H__INCLUDED
#define XEFIS__UTILITY__EVENT_TIMESTAMPER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>
#include <optional>
#include <functional>


namespace xf {

class EventTimestamper
{
  public:
	/**
	 * Update to current timestamp and test the condition.
	 */
	void
	update (si::Time now, std::function<bool()> condition);

	/**
	 * True if the condition was true during last update() call.
	 */
	bool
	condition() const noexcept;

	/**
	 * Time from the last event (aka when condition became true) to the last
	 * call of update() (aka now). Empty std::optional if condition() is
	 * false.
	 */
	std::optional<si::Time>
	stretch() const noexcept;

	/**
	 * True if the time since the event is less or equal to given argument.
	 */
	bool
	shorter_than (si::Time) const noexcept;

  private:
	bool						_last_test	{ false };
	std::optional<si::Time>		_timestamp;
	si::Time					_now;
};


inline void
EventTimestamper::update (si::Time now, std::function<bool()> condition)
{
	bool const new_test = condition();
	_now = now;

	if (new_test && !_last_test)
		_timestamp = now;

	_last_test = new_test;
}


inline bool
EventTimestamper::condition() const noexcept
{
	return _last_test;
}


inline std::optional<si::Time>
EventTimestamper::stretch() const noexcept
{
	if (_timestamp)
		return _now - *_timestamp;
	else
		return std::nullopt;
}


inline bool
EventTimestamper::shorter_than (si::Time time) const noexcept
{
	auto const s = stretch();

	return s && *s < time;
}

} // namespace xf

#endif

