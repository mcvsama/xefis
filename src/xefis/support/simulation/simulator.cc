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
#include "simulator.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

Simulator::Simulator (rigid_body::System& rigid_body_system,
					  rigid_body::ImpulseSolver& rigid_body_solver,
					  si::Time const initial_simulation_time,
					  si::Time const frame_duration,
					  nu::Logger const& logger):
	_logger (logger),
	_rigid_body_system (rigid_body_system),
	_rigid_body_solver (rigid_body_solver)
{
	_evolver.emplace (initial_simulation_time, frame_duration, logger.with_context ("Evolver"), [this] (si::Time const dt) {
		_rigid_body_solver.evolve (dt);
	});
}

} // namespace xf



