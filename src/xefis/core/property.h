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
		 * Create a null property, that can't be read or written.
		 */
		Property();

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
		 * Return true if property is nil.
		 */
		bool
		is_nil() const;

		/**
		 * Inverse of is_nil().
		 */
		bool
		valid() const;

		/**
		 * Set property to the nil value.
		 */
		void
		set_nil();

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
		write (Type);

		/**
		 * Write to a property. If node can't be found, throw PropertyNotFound.
		 */
		void
		write_signalling (Type);

		/**
		 * Return property path.
		 */
		std::string
		path() const;

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
	Property<T>::Property():
		_root (nullptr)
	{ }


template<class T>
	inline
	Property<T>::Property (Property const& other):
		_root (other._root),
		_path (other._path)
	{ }


template<class T>
	inline
	Property<T>::Property (const char* path):
		Property (std::string (path))
	{ }


template<class T>
	inline
	Property<T>::Property (std::string const& path):
		_root (PropertyStorage::root()),
		_path (path)
	{
		if (!_root)
			throw std::logic_error ("PropertyStorage is not initialized, can't construct Property with default storage");
	}


template<class T>
	inline
	Property<T>::Property (PropertyNode* node, std::string const& path):
		_root (node->root()),
		_path (path)
	{ }


template<class T>
	inline Property<T>&
	Property<T>::operator= (Property const& other)
	{
		_root = other._root;
		_path = other._path;
		return *this;
	}


template<class T>
	inline bool
	Property<T>::is_nil() const
	{
		if (!_root)
			throw std::logic_error ("can't read the root property");
		PropertyNode* node = _root->locate (_path);
		if (!node)
			return true;
		return node->is_nil();
	}


template<class T>
	inline bool
	Property<T>::valid() const
	{
		return !is_nil();
	}


template<class T>
	inline void
	Property<T>::set_nil()
	{
		if (!_root)
			throw std::logic_error ("can't write to the root property");
		PropertyNode* node = _root->locate (_path);
		if (node)
			node->set_nil();
	}


template<class T>
	inline typename
	Property<T>::Type
	Property<T>::read() const
	{
		if (!_root)
			throw std::logic_error ("can't read the root property");
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
		if (!_root)
			throw std::logic_error ("can't read the root property");
		PropertyNode* node = _root->locate (_path);
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
	Property<T>::write (Type value)
	{
		if (!_root)
			throw std::logic_error ("can't write to the root property");
		PropertyNode* node = _root->locate (_path);
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
	Property<T>::write_signalling (Type value)
	{
		if (!_root)
			throw std::logic_error ("can't write to the root property");
		PropertyNode* node = _root->locate (_path);
		if (!node)
			throw PropertyNotFound ("could not find property by path");
		node->write<Type> (value);
	}


template<class T>
	inline std::string
	Property<T>::path() const
	{
		return _path;
	}

} // namespace Xefis

#endif

