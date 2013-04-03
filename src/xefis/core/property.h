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
	is_singular() const noexcept;

	/**
	 * Valid means not singular and not nil.
	 */
	bool
	valid() const noexcept;

	/**
	 * Return property path.
	 */
	std::string const&
	path() const noexcept;

	/**
	 * Return actual property type.
	 */
	PropertyType
	real_type() const noexcept;

  protected:
	/**
	 * Return proper node. If cached node's path
	 * matches this property's path, return it.
	 * Otherwise, locate it.
	 */
	PropertyNode*
	get_node() const;

	/**
	 * Normalize path so if there's "//" in it,
	 * it will be replaced by leading "/".
	 */
	static std::string
	normalized_path (std::string path);

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
		read (Type default_value = Type()) const;

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

		/**
		 * Copy other node. Call PropertyNode::copy().
		 */
		void
		copy (Property const& other);

	  protected:
		/**
		 * Ensure that the property exists in the tree.
		 */
		template<class V>
			PropertyNode*
			ensure_path (std::string const& path, V value);
	};


inline
PropertyNotFound::PropertyNotFound (const char* message):
	std::runtime_error (message)
{ }


inline
BaseProperty::BaseProperty (PropertyNode* root, std::string path):
	_root (root),
	_path (normalized_path (path))
{ }


inline std::string
BaseProperty::normalized_path (std::string path)
{
	std::size_t p = path.rfind ("//");
	if (p != std::string::npos)
		return path.substr (p + 1);
	return path;
}


inline bool
BaseProperty::is_nil() const
{
	if (_root)
	{
		PropertyNode* node = get_node();
		if (node)
			return node->is_nil();
		return true;
	}
	else
		throw Exception ("can't read from a singular property");
}


inline void
BaseProperty::set_nil()
{
	if (_root)
	{
		PropertyNode* node = get_node();
		if (node)
			node->set_nil();
	}
	else
		throw Exception ("can't write to a singular property");
}


inline bool
BaseProperty::is_singular() const noexcept
{
	return !_root;
}


inline bool
BaseProperty::valid() const noexcept
{
	return !is_singular() && !is_nil();
}


inline std::string const&
BaseProperty::path() const noexcept
{
	return _path;
}


inline PropertyType
BaseProperty::real_type() const noexcept
{
	if (_root)
	{
		PropertyNode* node = get_node();
		if (node)
			return node->type();
		throw Exception ("can't check real type of nonexistent property");
	}
	else
		throw Exception ("can't check type of a singular property");
}


inline PropertyNode*
BaseProperty::get_node() const
{
	if (_root)
	{
		if (_node && _node->path() == _path)
			return _node;

		// Recache:
		return _node = _root->locate (_path);
	}
	else
		return nullptr;
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
	Property<T>::read (Type default_value) const
	{
		if (_root)
		{
			PropertyNode* node = get_node();
			if (node)
				return node->read<T> (default_value);
		}
		return default_value;
	}


template<class T>
	inline typename
	Property<T>::Type
	Property<T>::read_signalling() const
	{
		if (_root)
		{
			PropertyNode* node = get_node();
			if (node)
				return node->read<T>();
			throw PropertyNotFound ("could not find property by path");
		}
		else
			throw Exception ("can't read from a singular property");
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
		if (_root)
		{
			PropertyNode* node = get_node();
			if (node)
				node->write<Type> (value);
			else
				ensure_path (_path, value);
		}
		else
			throw Exception ("can't write to a singular property");
	}


template<class T>
	inline void
	Property<T>::write_signalling (Type const& value)
	{
		if (_root)
		{
			PropertyNode* node = get_node();
			if (node)
				node->write<Type> (value);
			else
				throw PropertyNotFound ("could not find property by path");
		}
		else
			throw Exception ("can't write to a singular property");
	}


template<class T>
	inline void
	Property<T>::copy (Property<T> const& other)
	{
		write (other.read());
		if (other.is_nil())
			set_nil();
		if (_root)
		{
			// Write the value to ensure that proper tree path
			// is created.
			PropertyNode* node = get_node();
			if (!node)
				node = ensure_path (_path, Type());
			PropertyNode* source = other.get_node();
			if (node && source)
				node->copy (*source);
		}
		else
			throw Exception ("can't copy to a singular property");
	}


template<class T>
	template<class V>
		inline PropertyNode*
		Property<T>::ensure_path (std::string const& path, V value)
		{
			std::string::size_type s = path.find_last_of ('/');
			std::string dir = path.substr (0, s);
			std::string pro = path.substr (s + 1);

			PropertyNode* parent = _root;
			if (s != std::string::npos)
				parent = _root->mkpath (dir);
			return parent->add_child (new PropertyNode (pro, value));
		}


/*
 * Shortcut types
 */


typedef Property<bool>			PropertyBoolean;
typedef Property<int>			PropertyInteger;
typedef Property<double>		PropertyFloat;
typedef Property<std::string>	PropertyString;

} // namespace Xefis

#include "si_property.h"

#endif

