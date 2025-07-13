/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
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
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/core/module.h>
#include <xefis/core/xefis.h>

// Neutrino:
#include <neutrino/time_helper.h>

// Lib:
#include <boost/circular_buffer.hpp>

// Standard:
#include <cstddef>
#include <functional>


namespace xf {

ProcessingLoop::InternalTimer::InternalTimer (ProcessingLoop& loop)
{
	timer.setSingleShot (false);
	timer.setTimerType (Qt::PreciseTimer);
	timer.setInterval (loop.period().in<si::Millisecond>());
	QObject::connect (&timer, &QTimer::timeout, [&loop] {
		loop.execute_cycle (nu::utc_now());
	});
}


ProcessingLoop::ProcessingLoop (std::string_view const instance, si::Frequency loop_frequency, nu::Logger const& logger):
	Module (instance),
	_loop_period (1.0 / loop_frequency),
	_logger (logger)
{
	_logger.set_logger_tag_provider (*this);
	register_module (*this);
}


void
ProcessingLoop::start()
{
	init_uninitialized_modules();
	switch_to_internal_timer().timer.start();
}


void
ProcessingLoop::stop()
{
	if (auto* int_timer = std::get_if<InternalTimer> (&_loop_timer))
		int_timer->timer.stop();
}


void
ProcessingLoop::advance (si::Time const duration)
{
	init_uninitialized_modules();

	auto& ext_timer = switch_to_external_timer();
	ext_timer.integrated_time += duration;

	while (ext_timer.integrated_time > _loop_period)
	{
		ext_timer.current_time += _loop_period;
		execute_cycle (ext_timer.current_time);
		ext_timer.integrated_time -= _loop_period;
	}
}


void
ProcessingLoop::set_external_timer_time (si::Time const time)
{
	switch_to_external_timer().current_time = time;
}


void
ProcessingLoop::init_uninitialized_modules()
{
	for (auto* module: _uninitialized_modules)
		Module::ModuleSocketAPI (*module).verify_settings();

	for (auto* module: _uninitialized_modules)
		module->initialize();

	_uninitialized_modules.clear();
}


ProcessingLoop::InternalTimer&
ProcessingLoop::switch_to_internal_timer()
{
	if (auto* int_timer = std::get_if<InternalTimer> (&_loop_timer))
		return *int_timer;
	else
		return _loop_timer.emplace<InternalTimer> (*this);
}


ProcessingLoop::ExternalTimer&
ProcessingLoop::switch_to_external_timer (si::Time const current_time)
{
	if (auto* ext_timer = std::get_if<ExternalTimer> (&_loop_timer))
		return *ext_timer;
	else
		return _loop_timer.emplace<ExternalTimer> (current_time, 0_s);
}


void
ProcessingLoop::execute_cycle (si::Time const now)
{
	if (!_previous_timestamp)
		_previous_timestamp = now - _loop_period;

	si::Time dt = now - *_previous_timestamp;
	si::Time latency = dt - _loop_period;

	_current_cycle = Cycle (_next_cycle_number++, now, dt, _loop_period, _logger);
	_processing_latencies.push_back (latency);
	this->latency = latency;
	this->actual_frequency = 1.0 / dt;

	for (auto* module: _modules)
		Module::ProcessingLoopAPI (*module).reset_cache();

	_communication_times.push_back (nu::measure_time ([this] {
		for (auto* module: _modules)
			Module::ProcessingLoopAPI (*module).communicate (*_current_cycle);
	}));

	_processing_times.push_back (nu::measure_time ([this] {
		for (auto* module: _modules)
		{
			Module::AccountingAPI (*module).set_cycle_time (period());
			Module::ProcessingLoopAPI (*module).fetch_and_process (*_current_cycle);
		}
	}));

	if (latency > kLatencyFactorLogThreshold * _loop_period)
		_logger << std::format ("Latency! {:.0f}% delay.\n", latency / _loop_period * 100.0);

	_previous_timestamp = now;
	_current_cycle.reset();
}


std::optional<std::string>
ProcessingLoop::logger_tag() const
{
	if (auto cycle = current_cycle())
		return std::format ("cycle={:08d}", cycle->number());
	else
		return "cycle=--------";
}

} // namespace xf

