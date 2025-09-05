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

// Local:
#include "simulator_machine.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine_manager.h>
#include <xefis/support/ui/paint_helper.h>

// Neutrino:
#include <neutrino/qt/qstring.h>
#include <neutrino/qt/qutils.h>

// Qt:
#include <QMenuBar>

// Standard:
#include <cstddef>
#include <memory>


namespace sim1::simulation {

[[nodiscard]]
QString
escape_qmenu_title (QString title)
{
	return title.replace ("&", "&&");
}


SimulatorMachine::SimulatorMachine (xf::Xefis& xefis):
	xf::Machine (xefis, u8"Sim-1 simulator")
{
	create_main_window();
}


void
SimulatorMachine::create_main_window()
{
	auto const ph = xf::PaintHelper (_main_window);

	auto* main_menu = new QMenuBar (&_main_window);
	auto* simulation_menu = main_menu->addMenu ("&Simulation");
	simulation_menu->addAction ("&Restart", [this] {
		restart_simulation();
	});

	{
		auto* restart_in_menu = simulation_menu->addMenu ("Restart &in");

		restart_in_menu->addAction ("Wrocław", [this] {
			restart_simulation (si::LonLatRadius (17.0386_deg, 51.1093_deg, _height));
		});
		restart_in_menu->addAction ("Oslo", [this] {
			restart_simulation (si::LonLatRadius (10.7522_deg, 59.9139_deg, _height));
		});
		restart_in_menu->addAction ("Angola", [this] {
			restart_simulation (si::LonLatRadius (-8.8147_deg, 13.2302_deg, _height));
		});
		restart_in_menu->addAction ("Ottawa", [this] {
			restart_simulation (si::LonLatRadius (-75.7003_deg, 45.4201_deg, _height));
		});
		restart_in_menu->addAction ("North pole (+Z)", [this] {
			restart_simulation (si::LonLatRadius (0_deg, 89.9999_deg, _height)); // FIXME at 0/90° exactly there are numerical errors
		});
		restart_in_menu->addAction ("South pole (-Z)", [this] {
			restart_simulation (si::LonLatRadius (0_deg, -89.9999_deg, _height)); // FIXME at 0/90° exactly there are numerical errors
		});
		restart_in_menu->addAction ("Null Island (+X)", [this] {
			restart_simulation (si::LonLatRadius (0_deg, 0_deg, _height));
		});
		restart_in_menu->addAction ("East (+Y)", [this] {
			restart_simulation (si::LonLatRadius (90_deg, 0_deg, _height));
		});
	}

	_machines_menu = main_menu->addMenu ("&Machines");

	_main_window.setMenuBar (main_menu);
	_main_window.resize (QSize (ph.em_pixels (80.0), ph.em_pixels (40.0)));
	_main_window.show();

	restart_simulation();
}


void
SimulatorMachine::setup_machines_menu()
{
	_machines_menu->clear();
	_machines_menu->setEnabled (!!_simulation_run);

	if (_simulation_run)
	{
		auto const add_machine_menu = [this] (xf::BasicMachineManager& machine_manager) {
			auto* machine_menu = _machines_menu->addMenu (escape_qmenu_title (neutrino::to_qstring (machine_manager.name())));

			{
				auto* show_configurator = machine_menu->addAction ("Show &configurator…", [this, &machine_manager] {
					if (auto* machine = machine_manager.machine())
						machine->show_configurator();
				});
				show_configurator->setEnabled (!!machine_manager.machine());
			}

			{
				auto* paused_action = machine_menu->addAction ("&Paused", [this, &machine_manager] {
					if (auto* machine = machine_manager.machine())
						machine->set_paused (!machine->paused());
				});
				paused_action->setCheckable (true);

				if (auto* machine = machine_manager.machine())
					paused_action->setChecked (machine->paused());
			}

			machine_menu->addSeparator();

			{
				auto* kill_action = machine_menu->addAction ("&Kill", [this, &machine_manager] {
					machine_manager.kill();
					setup_machines_menu();
				});
				kill_action->setEnabled (!!machine_manager.machine());
			}

			machine_menu->addAction ("&Restart", [this, &machine_manager] {
				machine_manager.restart();
				setup_machines_menu();
			});
		};

		for (auto machine_manager: _simulation_run->simulator().machine_managers())
			add_machine_menu (*machine_manager);
	}
}


void
SimulatorMachine::restart_simulation (std::optional<si::LonLatRadius<>> const location)
{
	if (location)
		_last_location = *location;

	_simulation_run.emplace (xefis(), _last_location, xefis().logger().with_context ("simulation"));
	// Note: Machine must be deleted first so that simulator widget unregisters itself from QMainWindow, before QMainWindow tries to delete it
	// in its destructor:
	_main_window.setCentralWidget (&_simulation_run->simulator_widget());
	setup_machines_menu();
}


} // namespace sim1::simulation
