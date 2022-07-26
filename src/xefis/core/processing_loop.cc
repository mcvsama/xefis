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

// Local:
#include "processing_loop.h"

// Xefis:
#include <xefis/app/xefis.h>
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/core/module.h>

// Neutrino:
#include <neutrino/time_helper.h>

// Lib:
#include <boost/circular_buffer.hpp>
#include <boost/format.hpp>

// Standard:
#include <cstddef>
#include <functional>


namespace xf {

ProcessingLoop::ProcessingLoop (Machine& machine, std::string_view const& instance, si::Frequency loop_frequency, Logger const& logger):
	ProcessingLoopIO (instance),
	_machine (machine),
	_xefis (machine.xefis()),
	_loop_period (1.0 / loop_frequency),
	_logger (logger)
{
	_loop_timer = new QTimer (this);
	_loop_timer->setSingleShot (false);
	_loop_timer->setTimerType (Qt::PreciseTimer);
	_loop_timer->setInterval (_loop_period.in<si::Millisecond>());
	QObject::connect (_loop_timer, &QTimer::timeout, this, &ProcessingLoop::execute_cycle);

	_logger.set_logger_tag_provider (*this);
}


ProcessingLoop::~ProcessingLoop()
{
	// The only allowed registered module during destruction is this ProcessingLoop itself:
	if (_modules_tracker.size() > 1 || (_modules_tracker.size() == 1 && &_modules_tracker.begin()->value() != this))
	{
		_logger << "Destroying ProcessingLoop. Registered modules:\n";

		for (auto const& disclosure: _modules_tracker)
			_logger << "  module " << identifier (disclosure.value()) << "\n";

		Exception::terminate ("ProcessingLoop destroyed while still having registered modules");
	}
}


void
ProcessingLoop::start()
{
	for (auto* module: _uninitialized_modules)
		Module::ModuleSocketAPI (*module).verify_settings();

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

		_current_cycle = Cycle (_next_cycle_number++, t, dt, _loop_period, _logger);
		_processing_latencies.push_back (latency);
		_io.latency = latency;
		_io.actual_frequency = 1.0 / dt;

		for (auto& module_details: _module_details_list)
			Module::ProcessingLoopAPI (module_details.module()).reset_cache();

		_communication_times.push_back (TimeHelper::measure ([this] {
			for (auto& module_details: _module_details_list)
				Module::ProcessingLoopAPI (module_details.module()).communicate (*_current_cycle);
		}));

		_processing_times.push_back (TimeHelper::measure ([this] {
			for (auto& module_details: _module_details_list)
			{
				auto& module = module_details.module();
				Module::AccountingAPI (module).set_cycle_time (period());
				Module::ProcessingLoopAPI (module).fetch_and_process (*_current_cycle);
			}
		}));

		if (latency > kLatencyFactorLogThreshold * _loop_period)
			_logger << boost::format ("Latency! %.0f%% delay.\n") % (latency / _loop_period * 100.0);
	}

	_previous_timestamp = t;
	_current_cycle.reset();
}


std::optional<std::string>
ProcessingLoop::logger_tag() const
{
	if (auto cycle = current_cycle())
		return (boost::format ("cycle=%08d") % cycle->number()).str();
	else
		return "cycle=--------";
}

} // namespace xf

