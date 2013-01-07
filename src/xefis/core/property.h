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
#include "property_storage.h"


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


class BaseProperty
{
  protected:
	BaseProperty() = default;
	BaseProperty (BaseProperty const&) = default;
	BaseProperty& operator= (BaseProperty const&) = default;

	// Ctor
	BaseProperty (PropertyNode* root, std::string path);

  public:
	/**
	 * Return true if property is nil.
	 */
	bool
	is_nil() const;

	/**
	 * Set property to the nil value.
	 */
	void
	set_nil();

	/**
	 * Return true if the property is singular,
	 * that is uninitialized.
	 */
	bool
	is_singular() const;

	/**
	 * Valid means not singular and not nil.
	 */
	bool
	valid() const;

	/**
	 * Return property path.
	 */
	std::string
	path() const;

  protected:
	/**
	 * Return proper node. If cached node's path
	 * matches this property's path, return it.
	 * Otherwise, locate it.
	 */
	PropertyNode*
	get_node() const;

  protected:
	PropertyNode*			_root = nullptr;
	mutable PropertyNode*	_node = nullptr;
	std::string				_path;
};


/**
 * A property reference. Doesn't hold the data, but only the path,
 * and queries property storage whenever needed.
 */
template<class tType>
	class Property: public BaseProperty
	{
	  public:
		typedef tType Type;

	  public:
		/**
		 * Create a singular property, that can't be read or written.
		 */
		Property() = default;

		/**
		 * Copy from other property.
		 */
		Property (Property const& other);

		/**
		 * Create a Property that belongs to Xefis::PropertyStorage::root()
		 * (the default storage) and is bound to given path.
		 * NOTE: The PropertyStorage must be initialized before attempting
		 * to use this constructor.
		 */
		Property (const char* path);

		/**
		 * Same as Property (const char*).
		 */
		Property (std::string const& path);

		/**
		 * Create a Property that belongs to a PropertyStorage
		 * bound to given path.
		 */
		Property (PropertyNode* root, std::string const& path);

		/**
		 * Copy from other property.
		 */
		Property&
		operator= (Property const& other);

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

		/**
		 * Unary * operator - same as read().
		 */
		Type
		operator*() const;

		/**
		 * Write to a property. If node can't be found, create it.
		 * If not possible, throw PropertyPathConflict.
		 */
		void
		write (Type const&);

		/**
		 * Write to a property. If node can't be found, throw PropertyNotFound.
		 */
		void
		write_signalling (Type const&);
	};


inline
PropertyNotFound::PropertyNotFound (const char* message):
	std::runtime_error (message)
{ }


inline
BaseProperty::BaseProperty (PropertyNode* root, std::string path):
	_root (root),
	_path (path)
{ }


inline bool
BaseProperty::is_nil() const
{
	if (!_root)
		throw Exception ("can't read a singular property");
	PropertyNode* node = get_node();
	if (!node)
		return true;
	return node->is_nil();
}


inline void
BaseProperty::set_nil()
{
	if (!_root)
		throw Exception ("can't write to a singular property");
	PropertyNode* node = get_node();
	if (node)
		node->set_nil();
}


inline bool
BaseProperty::is_singular() const
{
	return !_root;
}


inline bool
BaseProperty::valid() const
{
	return !is_singular() && !is_nil();
}


inline std::string
BaseProperty::path() const
{
	return _path;
}


inline PropertyNode*
BaseProperty::get_node() const
{
	if (!_root)
		return nullptr;

	if (_node && _node->path() == _path)
		return _node;

	// Recache:
	return _node = _root->locate (_path);
}


template<class T>
	inline
	Property<T>::Property (Property const& other):
		BaseProperty (other)
	{ }


template<class T>
	inline
	Property<T>::Property (const char* path):
		Property (std::string (path))
	{ }


template<class T>
	inline
	Property<T>::Property (std::string const& path):
		BaseProperty (PropertyStorage::default_storage()->root(), path)
	{
		if (!_root)
			throw Exception ("PropertyStorage is not initialized, can't construct Property with default storage");
	}


template<class T>
	inline
	Property<T>::Property (PropertyNode* node, std::string const& path):
		BaseProperty (node->root(), path)
	{ }


template<class T>
	inline Property<T>&
	Property<T>::operator= (Property const& other)
	{
		BaseProperty::operator= (other);
		return *this;
	}


template<class T>
	inline typename
	Property<T>::Type
	Property<T>::read() const
	{
		if (!_root)
			throw Exception ("can't read a singular property");
		PropertyNode* node = get_node();
		if (!node)
			return Type();
		return node->read<T>();
	}


template<class T>
	inline typename
	Property<T>::Type
	Property<T>::read_signalling() const
	{
		if (!_root)
			throw Exception ("can't read a singular property");
		PropertyNode* node = get_node();
		if (!node)
			throw PropertyNotFound ("could not find property by path");
		return node->read<T>();
	}


template<class T>
	inline typename
	Property<T>::Type
	Property<T>::operator*() const
	{
		return read();
	}


template<class T>
	inline void
	Property<T>::write (Type const& value)
	{
		if (!_root)
			throw Exception ("can't write to a singular property");
		PropertyNode* node = get_node();
		if (!node)
		{
			std::string::size_type s = _path.find_last_of ('/');
			std::string dir = _path.substr (0, s);
			std::string pro = _path.substr (s + 1);

			PropertyNode* parent = _root;
			if (s != std::string::npos)
				parent = _root->mkpath (dir);
			node = parent->add_child (new PropertyNode (pro, value));
		}
		else
			node->write<Type> (value);
	}


template<class T>
	inline void
	Property<T>::write_signalling (Type const& value)
	{
		if (!_root)
			throw Exception ("can't write to a singular property");
		PropertyNode* node = get_node();
		if (!node)
			throw PropertyNotFound ("could not find property by path");
		node->write<Type> (value);
	}


/*
 * Shortcut types
 */


typedef Property<bool>			PropertyBoolean;
typedef Property<int>			PropertyInteger;
typedef Property<double>		PropertyFloat;
typedef Property<std::string>	PropertyString;

} // namespace Xefis

#endif

