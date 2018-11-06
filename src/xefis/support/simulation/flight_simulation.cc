/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
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
#include <xefis/support/math/space.h>
#include <xefis/support/nature/physics.h>
#include <xefis/support/simulation/body_functions.h>

// Local:
#include "flight_simulation.h"


namespace xf::sim {

FlightSimulation::FlightSimulation (Airframe&& airframe, si::Frequency update_frequency, Logger const& logger):
	_logger (logger),
	_frame_dt (1 / update_frequency),
	_airframe (std::move (airframe)),
	_earth (make_earth())
{ }


void
FlightSimulation::evolve (si::Time dt, si::Time dt_limit)
{
	si::Time real_time_taken = 0_s;

	_real_time += dt;

	while (_simulation_time < _real_time)
	{
		auto const real_frame_start = TimeHelper::now();

		_airframe_forces = _airframe.forces (_atmosphere);

		auto forces = n_body_problem_forces<ECEFFrame> (_bodies.begin(), _bodies.end());
		forces[kEarthIndex] -= _airframe_forces;
		forces[kAirframeIndex] += _airframe_forces;

		for (auto& force_torque: forces)
		{
			// TODO Gentle limiting slope:
			force_torque.set_force (length_limited (force_torque.force(), kMaxForce));
			force_torque.set_torque (length_limited (force_torque.torque(), kMaxTorque));
		}

		for (auto const& body: _bodies | boost::adaptors::indexed())
			body.value()->act (forces[body.index()], _frame_dt);

		for (auto& body: _bodies)
		{
			// TODO Gentle limiting slope:
			body->set_velocity (length_limited (body->velocity(), kMaxVelocity));
			body->set_angular_velocity (length_limited<si::BaseAngularVelocity> (body->angular_velocity(), convert (kMaxAngularVelocity)));
		}

		// Auto-throttle the simulation if we can't fit into required dt_limit:
		auto const real_frame_end = TimeHelper::now();
		real_time_taken += real_frame_end - real_frame_start;

		if (real_time_taken >= dt_limit)
		{
			_logger << "Simulation throttled: skipping " << (_real_time - _simulation_time) << " of real time." << std::endl;
			_simulation_time = _real_time;
		}
		else
			_simulation_time += _frame_dt;
	}
}

} // namespace xf::sim

