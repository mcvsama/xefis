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
#include "property_storage.h"
#include "property.h"


namespace Xefis {

PropertyNode* PropertyStorage::_root = nullptr;


void
PropertyStorage::initialize()
{
	_root = new PropertyNode ("");
}


PropertyNode*
PropertyStorage::root()
{
	return _root;
}

} // namespace Xefis

