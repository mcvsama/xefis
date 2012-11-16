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

#ifndef XEFIS__CORE__PROPERTY_H__INCLUDED
#define XEFIS__CORE__PROPERTY_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>
#include <string>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property_node.h"


namespace Xefis {

/**
 * Indicates that property tried to be read
 * could not be found in the node tree.
 */
class PropertyNotFound: public std::runtime_error
{
  public:
	PropertyNotFound (const char* message);
};


/**
 * A property reference. Doesn't hold the data, but only the path,
 * and queries property storage whenever needed.
 */
template<class tType>
	class Property
	{
	  public:
		typedef tType Type;

	  public:
		/**
		 * Create a Property that belongs to a PropertyStorage
		 * bound to given path.
		 */
		Property (PropertyNode* root, std::string const& path);

		/**
		 * Read property. If node can't be found, return default value.
		 */
		Type
		read() const;

		/**
		 * Read property. If node can't be found, throw PropertyNotFound.
		 */
		Type
		read_signalling() const;

	  private:
		PropertyNode*	_root;
		std::string		_path;
	};


inline
PropertyNotFound::PropertyNotFound (const char* message):
	std::runtime_error (message)
{ }


template<class T>
	inline
	Property<T>::Property (PropertyNode* node, std::string const& path):
		_root (node->root()),
		_path (path)
	{ }


template<class T>
	inline typename
	Property<T>::Type
	Property<T>::read() const
	{
		PropertyNode* node = _root->locate (_path);
		if (!node)
			return Type();
		return node->read<T>();
	}


template<class T>
	inline typename
	Property<T>::Type
	Property<T>::read_signalling() const
	{
		PropertyNode* node = _root->locate (_path);
		if (!node)
			throw PropertyNotFound ("could not find property by path");
		return node->read<T>();
	}

} // namespace Xefis

#endif

