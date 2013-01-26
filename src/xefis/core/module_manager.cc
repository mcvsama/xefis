/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include <modules/computers/lookahead.h>
#include <modules/generic/property_tree.h>
#include <modules/io/flight_gear.h>
#include <modules/io/joystick.h>
#include <modules/instruments/efis.h>
#include <modules/instruments/hsi.h>
#include <modules/instruments/radial_indicator.h>

// Local:
#include "module_manager.h"


namespace Xefis {

ModuleManager::ModuleManager (Application* application):
	_application (application)
{ }


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
		module = new EFIS (this, config, parent);
		_modules.insert (module);
	}
	else if (name == "instruments/hsi")
	{
		module = new HSI (this, config, parent);
		_modules.insert (module);
	}
	else if (name == "instruments/radial-indicator")
	{
		module = new RadialIndicator (this, config, parent);
		_modules.insert (module);
	}
	else if (name == "computers/lookahead")
	{
		module = new Lookahead (this, config);
		_modules.insert (module);
	}
	else if (name == "generic/property-tree")
	{
		module = new PropertyTree (this, config, parent);
		_modules.insert (module);
	}
	else if (name == "io/flightgear")
	{
		module = new FlightGearIO (this, config);
		_modules.insert (module);
	}
	else if (name == "io/joystick")
	{
		module = new JoystickInput (this, config);
		_modules.insert (module);
	}
	else
		throw ModuleNotFoundException ("module not found: " + name.toStdString());

	return module;
}


void
ModuleManager::data_update (Timestamp timestamp)
{
	_update_dt = timestamp - _update_timestamp;
	_update_timestamp = timestamp;

	for (Module* mod: _modules)
		mod->data_update();
}

} // namespace Xefis

