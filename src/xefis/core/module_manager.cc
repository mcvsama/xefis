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

// Local:
#include "module_manager.h"
#include "module.h"


namespace Xefis {

ModuleManager::~ModuleManager()
{
	for (Module* m: _modules)
		delete m;
}


void
ModuleManager::add_module (Module* module)
{
	_modules.insert (module);
}

} // namespace Xefis

