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
#include "simulation.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

Simulation::Simulation (si::Frequency const world_frequency, Logger const& logger, Evolve const evolve):
	_logger (logger),
	_frame_dt (1 / world_frequency),
	_evolve (evolve)
{
	if (!_evolve)
		throw InvalidArgument ("'evolve' paramter must not be nullptr");
}


void
Simulation::evolve (si::Time dt, si::Time real_time_limit)
{
	si::Time real_time_taken = 0_s;

	_real_time += dt;

	while (_simulation_time < _real_time)
	{
		real_time_taken += TimeHelper::measure ([&] {
			_evolve (_frame_dt);
		});

		if (real_time_taken >= real_time_limit)
		{
			_logger << "Simulation throttled: skipping " << (_real_time - _simulation_time) << " of real time." << std::endl;
			_simulation_time = _real_time;
		}
		else
			_simulation_time += _frame_dt;
	}
}

} // namespace xf

