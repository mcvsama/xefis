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
#include <functional>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/xefis.h>
#include <xefis/core/v2/compatibility_v1_v2.h>
#include <xefis/core/v2/machine.h>
#include <xefis/core/v2/module.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "processing_loop.h"


namespace v2 {

ProcessingLoop::ProcessingLoop (Machine* machine, Frequency loop_frequency):
	_machine (machine),
	_xefis (machine->xefis()),
	_loop_period (1.0 / loop_frequency)
{
	_loop_timer = new QTimer (this);
	_loop_timer->setSingleShot (false);
	_loop_timer->setInterval (_loop_period.quantity<Millisecond>());
	QObject::connect (_loop_timer, &QTimer::timeout, this, &ProcessingLoop::execute_cycle);
}


void
ProcessingLoop::start()
{
	for (auto* module: _uninitialized_modules)
		Module::ProcessingLoopAPI (module).verify_settings();

	for (auto* module: _uninitialized_modules)
		module->initialize();

	_uninitialized_modules.clear();
	_loop_timer->start();
}


void
ProcessingLoop::stop()
{
	_loop_timer->stop();
}


void
ProcessingLoop::execute_cycle()
{
	Time t = TimeHelper::now();
	Time dt = t - _previous_timestamp.value_or (t - 1_ms); // -1_ms to prevent division by zero in modules.
	Cycle cycle { t, dt };

	//XXX std::cout << "--- processing loop ---\n";

	if (_previous_timestamp)
	{
		Time latency = dt - _loop_period;

		_latency = latency;
		_actual_frequency = 1.0 / dt;

		if (dt > 1.1 * _loop_period)
		{
			// TODO log:
			std::cout << boost::format ("Latency! %.0f%% delay.\n") % (dt / _loop_period * 100.0);
		}
	}

	// TODO check if all core properties are computable by modules; if not, show a warning.

	// TODO make lists of connected v1 and v2 properties

	compatibility_input();

	for (auto& module: _modules)
		v2::Module::ProcessingLoopAPI (module.get()).reset_cache();

	// TODO module accounting
	for (auto& module: _modules)
		v2::Module::ProcessingLoopAPI (module.get()).fetch_and_process (cycle);

	compatibility_output();

	_previous_timestamp = t;
}


void
ProcessingLoop::compatibility_input()
{
	// Copy all xf::Property<T> values to v2::PropertyIn/Out<T> objects.
	for (auto& copy: g_copy_to_v2)
		copy();
}


void
ProcessingLoop::compatibility_output()
{
	// Copy all v2::PropertyIn/Out<T> values to xf::Property<T> objects.
	for (auto& copy: g_copy_to_v1)
		copy();
}

} // namespace v2

