/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
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


namespace v1 {
using namespace xf;

Unique<PropertyStorage> PropertyStorage::_default_storage;


PropertyStorage::~PropertyStorage()
{
	delete _root;
}


void
PropertyStorage::initialize()
{
	_default_storage = std::make_unique<PropertyStorage>();
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
	return _default_storage.get();
}


PropertyNode*
PropertyStorage::locate (PropertyPath const& path) const
{
	auto it = _properties_by_path.find (path.string());
	if (it == _properties_by_path.end())
		return nullptr;
	return it->second;
}


void
PropertyStorage::cache_path (PropertyNode* node)
{
	_properties_by_path[node->path().string()] = node;
}


void
PropertyStorage::uncache_path (PropertyPath const& old_path)
{
	_properties_by_path.erase (old_path.string());
}

} // namespace v1

