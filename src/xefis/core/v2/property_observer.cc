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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property_observer.h"


namespace v2 {

void
PropertyObserver::observe (BasicProperty& property)
{
	_objects.push_back (Object (&property));
}


void
PropertyObserver::observe (PropertyObserver& observer)
{
	_objects.push_back (Object (&observer));
}


void
PropertyObserver::observe (std::initializer_list<Object> list)
{
	_objects.insert (_objects.end(), list.begin(), list.end());
}


void
PropertyObserver::set_callback (Callback callback) noexcept
{
	_callback = callback;
}


void
PropertyObserver::set_minimum_dt (Time dt) noexcept
{
	_minimum_dt = dt;
}


void
PropertyObserver::process (Time update_time)
{
	Time obs_dt = update_time - _obs_update_time;
	_accumulated_dt += update_time - _fire_time;

	for (Object& o: _objects)
	{
		BasicProperty::Serial new_serial = o.remote_serial();
		if (new_serial != o._saved_serial)
		{
			_need_callback = true;
			_last_recompute = !_smoothers.empty();
			o._saved_serial = new_serial;
		}
	}

	// Minimum time (granularity) for updates caused by working smoothers - 1 ms.
	bool should_recompute = _need_callback || (obs_dt >= 1_ms && obs_dt <= longest_smoothing_time());

	if (!should_recompute && _last_recompute)
	{
		should_recompute = true;
		_last_recompute = false;
	}

	if (should_recompute || _touch)
	{
		if (_accumulated_dt >= _minimum_dt)
		{
			if (_need_callback)
				_obs_update_time = update_time;
			_need_callback = false;
			_touch = false;
			_accumulated_dt = 0_ms;
			_fire_dt = update_time - _fire_time;
			_fire_time = update_time;
			++_serial;
			_callback();
		}
		else
			_last_recompute = true;
	}
}


void
PropertyObserver::add_depending_smoother (SmootherBase& smoother)
{
	_smoothers.push_back (&smoother);
	_recompute_longest_smoother = true;
}


void
PropertyObserver::add_depending_smoothers (std::initializer_list<SmootherBase*> list)
{
	_smoothers.insert (_smoothers.end(), list.begin(), list.end());
	_recompute_longest_smoother = true;
}


Time
PropertyObserver::longest_smoothing_time() noexcept
{
	if (_recompute_longest_smoother)
	{
		Time longest = 0_s;
		for (auto const& smoother: _smoothers)
			longest = std::max (longest, smoother->smoothing_time());
		// Add 1.1 ms of margin, to be sure that the smoother's window
		// is positioned _after_ the last interesting value change.
		// This assumes that the smoother's precision is set to 1 ms.
		_longest_smoother = longest + 1.1_ms;
		_longest_smoother *= 2.0;
		_recompute_longest_smoother = false;
	}

	return _longest_smoother;
}

} // namespace v2

