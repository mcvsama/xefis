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

#ifndef XEFIS__CORE__PROPERTY_STORAGE_H__INCLUDED
#define XEFIS__CORE__PROPERTY_STORAGE_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property_node.h"


namespace Xefis {

/**
 * Storage class for properties.
 */
class PropertyStorage
{
  public:
	/**
	 * Initialize storage.
	 */
	static void
	initialize();

	/**
	 * Return top-level PropertyNode of this storage.
	 */
	static PropertyNode*
	root();

  private:
	static PropertyNode* _root;
};

} // namespace Xefis

#endif

