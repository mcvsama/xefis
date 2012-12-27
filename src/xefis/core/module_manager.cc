/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <xefis/core/module.h>

// Modules:
#include <io/flight_gear.h>
#include <io/joystick.h>
#include <instruments/efis.h>
#include <instruments/power_indicator.h>

// Local:
#include "module_manager.h"


namespace Xefis {

ModuleManager::~ModuleManager()
{
	for (Module* m: _modules)
		delete m;
}


Module*
ModuleManager::load_module (QString const& name, QDomElement const& config, QWidget* parent)
{
	Module* module;

	if (name == "instruments/efis")
	{
		module = new EFIS (config, parent);
		_modules.insert (module);
	}
	else if (name == "instruments/power-indicator")
	{
		module = new PowerIndicator (config, parent);
		_modules.insert (module);
	}
	else if (name == "io/flightgear")
	{
		module = new FlightGearIO (config);
		_modules.insert (module);
	}
	else if (name == "io/joystick")
	{
		module = new JoystickInput (config);
		_modules.insert (module);
	}
	else
		throw ModuleNotFoundException ("module not found: " + name.toStdString());

	return module;
}

} // namespace Xefis

