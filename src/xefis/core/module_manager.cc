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
#include <typeinfo>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>

// Modules:
#include <modules/generic/fps.h>
#include <modules/generic/property_tree.h>
#include <modules/instruments/efis.h>
#include <modules/instruments/hsi.h>
#include <modules/instruments/radial_indicator.h>
#include <modules/io/flight_gear.h>
#include <modules/io/joystick.h>
#include <modules/systems/flight_director.h>
#include <modules/systems/fly_by_wire.h>
#include <modules/systems/fms.h>
#include <modules/systems/lookahead.h>
#include <modules/systems/mouse.h>

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
	Module* module = create_module_by_name (name, config, parent);
	_modules.insert (module);

	Instrument* instrument = dynamic_cast<Instrument*> (module);
	if (instrument)
		_instrument_modules.insert (instrument);
	else
		_non_instrument_modules.insert (module);

	return module;
}


void
ModuleManager::data_updated (Time time)
{
	_update_dt = 1_s * (time.s() - _update_time.s());
	if (_update_dt > 1.0_s)
		_update_dt = Time::epoch() + 1_s;

	_update_time = time;

	for (Module* mod: _non_instrument_modules)
		module_data_updated (mod);

	// Let instruments display data already computed by all other modules.
	// Also limit FPS of the instrument modules.
	if (time - _instrument_update_time > 1_s / 30.f)
	{
		for (Module* mod: _instrument_modules)
			module_data_updated (mod);
		_instrument_update_time = time;
	}
	else
	{
		// In case of inhibited instruments update, postpone the update a bit:
		application()->postponed_data_updated();
	}
}


Module*
ModuleManager::create_module_by_name (QString const& name, QDomElement const& config, QWidget* parent)
{
	try {
		if (name == "instruments/efis")
			return new EFIS (this, config, parent);
		else if (name == "instruments/hsi")
			return new HSI (this, config, parent);
		else if (name == "instruments/radial-indicator")
			return new RadialIndicator (this, config, parent);
		else if (name == "systems/lookahead")
			return new Lookahead (this, config);
		else if (name == "systems/mouse")
			return new Mouse (this, config);
		else if (name == "systems/flight-director")
			return new FlightDirector (this, config);
		else if (name == "systems/fly-by-wire")
			return new FlyByWire (this, config);
		else if (name == "systems/fms")
			return new FlightManagementSystem (this, config);
		else if (name == "generic/property-tree")
			return new PropertyTree (this, config, parent);
		else if (name == "generic/fps")
			return new FPS (this, config);
		else if (name == "io/flightgear")
			return new FlightGearIO (this, config);
		else if (name == "io/joystick")
			return new JoystickInput (this, config);
		else
			throw ModuleNotFoundException ("module not found: " + name.toStdString());
	}
	catch (Exception& e)
	{
		throw Exception ("error when loading module "_str + name.toStdString(), &e);
	}
}


void
ModuleManager::module_data_updated (Module* module)
{
	try {
		module->data_updated();
	}
	catch (Xefis::Exception const& e)
	{
		std::cerr << "Exception when processing update from module '" << typeid (*module).name() << "'" << std::endl;
		std::cerr << e << std::endl;
	}
}

} // namespace Xefis

