/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
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
#include "evolver.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

Evolver::Evolver (si::Time const frame_duration, Logger const& logger, Evolve const evolve):
	_logger (logger),
	_frame_duration (frame_duration),
	_evolve (evolve)
{
	if (!_evolve)
		throw InvalidArgument ("'evolve' paramter must not be nullptr");
}


Evolver::EvolutionResult
Evolver::evolve (si::Time const duration)
{
	_target_time += duration;
	auto frames = 0u;
	auto const prev_simulation_time = _simulation_time;
	auto const real_time_taken = TimeHelper::measure ([&] {
		while (_simulation_time < _target_time)
		{
			_evolve (_frame_duration);
			_simulation_time += _frame_duration;
			++frames;
		}
	});

	_performance = (_simulation_time - prev_simulation_time) / real_time_taken;

	return {
		.real_time_taken = real_time_taken,
		.evolved_frames = frames,
	};
}


Evolver::EvolutionResult
Evolver::evolve (std::size_t const frames)
{
	auto const prev_simulation_time = _simulation_time;
	auto const real_time_taken = TimeHelper::measure ([&] {
		for (std::size_t i = 0; i < frames; ++i)
		{
			_evolve (_frame_duration);
			_target_time += _frame_duration;
			_simulation_time += _frame_duration;
		}
	});

	_performance = (_simulation_time - prev_simulation_time) / real_time_taken;

	return {
		.real_time_taken = real_time_taken,
		.evolved_frames = frames,
	};
}

} // namespace xf

