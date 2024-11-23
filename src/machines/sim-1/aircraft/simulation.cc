/* vim:ts=4
 *
 * Copyleft 2008…2023  Michał Gawron
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
#include "machine.h"
#include "data_center.h"
#include "models.h"
#include "simulation.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/lonlat_radius.h>
#include <xefis/support/math/tait_bryan_angles.h>
#include <xefis/support/math/rotations.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/simulation/rigid_body/utility.h>
#include <xefis/xefis_machine.h>

// Neutrino:
#include <neutrino/math/math.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

Simulation::Simulation (Machine& machine, Models& models, neutrino::Logger const& logger):
	_logger (logger),
	_models (models),
	_aircraft (make_aircraft (_rigid_body_system, _models))
{
	auto const location = xf::LonLatRadius (0_deg, 45_deg, xf::kEarthMeanRadius + 0.5_km);
	auto const tait_bryan_angles = xf::TaitBryanAngles ({ .roll = 0_deg, .pitch = -30_deg, .yaw = 0_deg });
	_aircraft.rigid_group.rotate_about_world_origin (math::reframe<xf::WorldSpace, xf::WorldSpace> (airframe_to_ecef_rotation (tait_bryan_angles, location)));
	_aircraft.rigid_group.translate (math::reframe<xf::WorldSpace, void> (xf::cartesian (location)));

	auto& earth = _rigid_body_system.add_gravitating (xf::rigid_body::make_earth());
	earth.set_label ("Earth");

	_rigid_body_solver.set_required_precision (1_N, 0.1_Nm);
	_rigid_body_solver.set_limits (xf::rigid_body::Limits {
		.max_force				= 1e6_N,
		.max_torque				= 1e6_Nm,
		.max_velocity			= 1e6_mps,
		.max_angular_velocity	= 1e6_radps,
	});

	_simulator.emplace (_rigid_body_system, _rigid_body_solver, 1_ms, _logger.with_context ("Simulator"));

	_simulator_widget.emplace (*_simulator, nullptr);
	_simulator_widget->set_machine (&machine);
	_simulator_widget->set_followed_body (&_aircraft.primary_body);
	_simulator_widget->set_planet (&earth);
	_simulator_widget->show();
}

} // namespace sim1::aircraft

