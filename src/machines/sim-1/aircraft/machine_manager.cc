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
	simulation_menu->addAction ("&Restart", [&] {
		restart_machine();
	});
	simulation_menu->addAction ("Show &configuration…", [&] {
		if (_machine)
			_machine->show_configurator();
	});

	_main_window->setMenuBar (main_menu);
	_main_window->resize (QSize (ph.em_pixels (80.0), ph.em_pixels (40.0)));
	_main_window->show();
	restart_machine();
}


void
MachineManager::restart_machine()
{
	_machine.emplace (xefis());
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

