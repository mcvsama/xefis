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

// Local:
#include "property_storage.h"
#include "property.h"


namespace Xefis {

PropertyStorage* PropertyStorage::_default_storage = nullptr;


void
PropertyStorage::initialize()
{
	_default_storage = new PropertyStorage();
}


PropertyStorage::PropertyStorage():
	_root (new PropertyDirectoryNode (this))
{ }


PropertyDirectoryNode*
PropertyStorage::root() const noexcept
{
	return _root;
}


PropertyStorage*
PropertyStorage::default_storage()
{
	return _default_storage;
}


PropertyNode*
PropertyStorage::locate (std::string const& path) const
{
	auto it = _properties_by_path.find (path);
	if (it == _properties_by_path.end())
		return nullptr;
	return it->second;
}


void
PropertyStorage::cache_path (PropertyNode* node)
{
	_properties_by_path[node->path()] = node;
}


void
PropertyStorage::uncache_path (std::string const& old_path)
{
	_properties_by_path.erase (old_path);
}

} // namespace Xefis

