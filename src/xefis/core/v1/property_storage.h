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

#ifndef XEFIS__CORE__PROPERTY_STORAGE_H__INCLUDED
#define XEFIS__CORE__PROPERTY_STORAGE_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/property_utils.h>
#include <xefis/utility/noncopyable.h>


namespace xf {

class PropertyNode;
class PropertyDirectoryNode;

/**
 * Storage class for properties.
 */
class PropertyStorage: public Noncopyable
{
	friend class PropertyNode;

	typedef std::map<std::string, PropertyNode*> PropertiesByPath;

  public:
	// Dtor
	~PropertyStorage();

	/**
	 * Initialize storage - create default storage.
	 */
	static void
	initialize();

	/**
	 * Create a storage object.
	 */
	PropertyStorage();

	/**
	 * Return top-level PropertyNode of this storage.
	 */
	PropertyDirectoryNode*
	root() const noexcept;

	/**
	 * Return pointer to the default storage.
	 */
	static PropertyStorage*
	default_storage();

	/**
	 * Try to find registered property by its path.
	 * Return nullptr if not found.
	 */
	PropertyNode*
	locate (PropertyPath const& path) const;

	/**
	 * Get map of all registered properties.
	 */
	PropertiesByPath const&
	properties_map() const;

  private:
	/**
	 * Cache a path for the node for quicker locate().
	 * Path will be read from the node itself.
	 */
	void
	cache_path (PropertyNode* node);

	/**
	 * Remove previously cached path.
	 */
	void
	uncache_path (PropertyPath const& old_path);

  private:
	static Unique<PropertyStorage>	_default_storage;
	PropertyDirectoryNode*			_root;
	PropertiesByPath				_properties_by_path;
};


inline
PropertyStorage::PropertiesByPath const&
PropertyStorage::properties_map() const
{
	return _properties_by_path;
}

} // namespace xf

#endif

