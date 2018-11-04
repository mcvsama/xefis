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

// Lib:
#include <boost/circular_buffer.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/core/module.h>
#include <xefis/core/xefis.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "processing_loop.h"


namespace xf {

ProcessingLoop::ProcessingLoop (Machine& machine, std::string_view const& instance, Frequency loop_frequency, Logger const& logger):
	Module (std::make_unique<ProcessingLoopIO> (instance), instance),
	_machine (machine),
	_xefis (machine.xefis()),
	_loop_period (1.0 / loop_frequency),
	_logger (logger)
{
	_loop_timer = new QTimer (this);
	_loop_timer->setSingleShot (false);
	_loop_timer->setTimerType (Qt::PreciseTimer);
	_loop_timer->setInterval (_loop_period.in<Millisecond>());
	QObject::connect (_loop_timer, &QTimer::timeout, this, &ProcessingLoop::execute_cycle);

	_logger.set_processing_loop (*this);
}


ProcessingLoop::~ProcessingLoop()
{
	// The only allowed registered module during destruction is this ProcessingLoop itself:
	if (_modules_tracker.size() > 1 || (_modules_tracker.size() == 1 && &_modules_tracker.begin()->value() != this))
		Exception::terminate ("ProcessingLoop destroyed while still having registered modules");
}


void
ProcessingLoop::start()
{
	for (auto* module: _uninitialized_modules)
		ModuleIO::ProcessingLoopAPI (*module->io_base()).verify_settings();

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
	si::Time t = TimeHelper::now();

	if (_previous_timestamp)
	{
		si::Time dt = t - *_previous_timestamp;
		si::Time latency = dt - _loop_period;

		_current_cycle = Cycle (_next_cycle_number++, t, dt, _logger);
		_processing_latencies.push_back (latency);
		io.latency = latency;
		io.actual_frequency = 1.0 / dt;

		for (auto& module_details: _module_details_list)
			BasicModule::ProcessingLoopAPI (module_details.module()).reset_cache();

		_communication_times.push_back (TimeHelper::measure ([this] {
			for (auto& module_details: _module_details_list)
				BasicModule::ProcessingLoopAPI (module_details.module()).communicate (*_current_cycle);
		}));

		_processing_times.push_back (TimeHelper::measure ([this] {
			for (auto& module_details: _module_details_list)
			{
				auto& module = module_details.module();
				BasicModule::AccountingAPI (module).set_cycle_time (period());
				BasicModule::ProcessingLoopAPI (module).fetch_and_process (*_current_cycle);
			}
		}));

		if (latency > kLatencyFactorLogThreshold * _loop_period)
			_logger << boost::format ("Latency! %.0f%% delay.\n") % (latency / _loop_period * 100.0);
	}

	_previous_timestamp = t;
	_current_cycle.reset();
}

} // namespace xf

