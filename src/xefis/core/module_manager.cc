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
#include <modules/systems/ap.h>
#include <modules/systems/lookahead.h>
#include <modules/systems/stabilizer.h>

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
ModuleManager::data_updated (Timestamp timestamp)
{
	_update_dt = timestamp - _update_timestamp;
	if (_update_dt.seconds() > 1.0)
		_update_dt = Timestamp::from_epoch (1);

	_update_timestamp = timestamp;

	for (Module* mod: _non_instrument_modules)
	{
		try {
			mod->data_updated();
		}
		catch (Xefis::Exception const& e)
		{
			std::cerr << "Exception when processing update from module '" << typeid (*mod).name() << "'" << std::endl;
			std::cerr << e << std::endl;
		}
	}

	// Let instruments display data already computed by all other modules.
	// Also limit FPS of the instrument modules.
	if ((timestamp - _instrument_update_timestamp).seconds() > 1.f / 30.f)
	{
		for (Module* mod: _instrument_modules)
			mod->data_updated();
		_instrument_update_timestamp = timestamp;
	}
}


Module*
ModuleManager::create_module_by_name (QString const& name, QDomElement const& config, QWidget* parent)
{
	if (name == "instruments/efis")
		return new EFIS (this, config, parent);
	else if (name == "instruments/hsi")
		return new HSI (this, config, parent);
	else if (name == "instruments/radial-indicator")
		return new RadialIndicator (this, config, parent);
	else if (name == "systems/lookahead")
		return new Lookahead (this, config);
	else if (name == "systems/stabilizer")
		return new Stabilizer (this, config);
	else if (name == "systems/ap")
		return new AP (this, config);
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

} // namespace Xefis

