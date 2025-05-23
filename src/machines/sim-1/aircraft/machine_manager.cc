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
#include "machine.h"
#include "machine_manager.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/paint_helper.h>

// Neutrino:
#include <neutrino/qt/qutils.h>

// Qt:
#include <QMenuBar>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

MachineManager::MachineManager (xf::Xefis& xefis):
	xf::MachineManager (xefis)
{
	create_main_window();
}


xf::Machine&
MachineManager::machine()
{
	return _machine.value();
}


void
MachineManager::create_main_window()
{
	_main_window.emplace();
	auto const ph = xf::PaintHelper (*_main_window);

	auto* main_menu = new QMenuBar (&*_main_window);
	auto* simulation_menu = main_menu->addMenu ("&Machine");
	simulation_menu->addAction ("&Restart", [this] {
		restart_machine();
	});

	{
		auto* restart_in_menu = simulation_menu->addMenu ("Restart &in");

		restart_in_menu->addAction ("Wrocław", [this] {
			restart_machine (si::LonLatRadius (17.0386_deg, 51.1093_deg, _height));
		});
		restart_in_menu->addAction ("Oslo", [this] {
			restart_machine (si::LonLatRadius (10.7522_deg, 59.9139_deg, _height));
		});
		restart_in_menu->addAction ("Angola", [this] {
			restart_machine (si::LonLatRadius (-8.8147_deg, 13.2302_deg, _height));
		});
		restart_in_menu->addAction ("Ottawa", [this] {
			restart_machine (si::LonLatRadius (-75.7003_deg, 45.4201_deg, _height));
		});
		restart_in_menu->addAction ("North pole (+Z)", [this] {
			restart_machine (si::LonLatRadius (0_deg, 89.9999_deg, _height)); // FIXME at 0/90° exactly there are numerical errors
		});
		restart_in_menu->addAction ("South pole (-Z)", [this] {
			restart_machine (si::LonLatRadius (0_deg, -89.9999_deg, _height)); // FIXME at 0/90° exactly there are numerical errors
		});
		restart_in_menu->addAction ("Null Island (+X)", [this] {
			restart_machine (si::LonLatRadius (0_deg, 0_deg, _height));
		});
		restart_in_menu->addAction ("East (+Y)", [this] {
			restart_machine (si::LonLatRadius (90_deg, 0_deg, _height));
		});
	}

	simulation_menu->addAction ("Show &configuration…", [this] {
		_machine.value().show_configurator();
	});

	_main_window->setMenuBar (main_menu);
	_main_window->resize (QSize (ph.em_pixels (80.0), ph.em_pixels (40.0)));
	_main_window->show();

	restart_machine();
}


void
MachineManager::restart_machine (std::optional<si::LonLatRadius<>> const location)
{
	if (location)
		_last_location = *location;

	_machine.emplace (xefis(), _last_location);
	// Note: Machine must be deleted first so that simulator widget unregisters itself from QMainWindow, before QMainWindow tries to delete it
	// in its destructor:
	_main_window->setCentralWidget (&_machine->simulator_widget());
}

} // namespace sim1::aircraft


std::unique_ptr<xf::MachineManager>
make_xefis_machine_manager (xf::Xefis& xefis)
{
	return std::make_unique<sim1::aircraft::MachineManager> (xefis);
}

