/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIM_1__SIMULATION__SIMULATOR_MACHINE_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__SIMULATION__SIMULATOR_MACHINE_H__INCLUDED

// Local:
#include "simulation_run.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/core/machine_manager.h>
#include <xefis/core/xefis.h>

// Neutrino:
#include <neutrino/si/lonlat_radius.h>

// Qt:
#include <QMainWindow>
#include <QMenu>

// Standard:
#include <cstddef>
#include <optional>


namespace sim1::simulation {

/**
 * Top level GUI application that manages simulations.
 */
class SimulatorMachine: public xf::Machine
{
  public:
	// Ctor
	SimulatorMachine (xf::Xefis&);

  private:
	void
	create_main_window();

	void
	setup_machines_menu();

	void
	restart_simulation (std::optional<si::LonLatRadius<>> const location = std::nullopt);

  private:
	si::Length						_height			{ xf::kEarthMeanRadius + 500_m };
	si::LonLatRadius<>				_last_location	{ 17.0386_deg, 51.1093_deg, _height };
	QMainWindow						_main_window;
	std::optional<SimulationRun>	_simulation_run;
	QMenu*							_machines_menu	{ nullptr };
};

} // namespace sim1::simulation

#endif
