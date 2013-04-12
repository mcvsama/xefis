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
class PropertyNotFound: public Exception
{
  public:
	PropertyNotFound (std::string const& message);
};


/**
 * Indicates that the property is singular (not attached to any tree)
 * and can't be written or read.
 */
class SingularProperty: public Exception
{
  public:
	SingularProperty (std::string const& message);
};


/**
 * Indicates that the operation is invalid on certain
 * node type.
 */
class InvalidOperation: public Exception
{
  public:
	InvalidOperation (std::string const& message);
};


class BaseProperty
{
  protected:
	BaseProperty() = default;
	BaseProperty (BaseProperty const&) = default;
	BaseProperty& operator= (BaseProperty const&) = default;

	// Ctor
	BaseProperty (PropertyDirectoryNode* root, std::string path);

	// Dtor
	virtual ~BaseProperty();

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
	 * Point this property to another PropertyNode.
	 */
	void
	set_path (std::string const&);

	/**
	 * Ensures that this property exists.
	 */
	void
	ensure_existence();

	/**
	 * Return humanized value (eg. value with unit).
	 * Default implementation uses read<string>().
	 */
	virtual std::string
	stringify() const = 0;

	/**
	 * Set value from humanized string (eg. "10 kt").
	 * Default implementation uses write().
	 */
	virtual void
	parse (std::string const&) = 0;

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
	PropertyDirectoryNode*	_root = nullptr;
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
		typedef tType					Type;
		typedef PropertyValueNode<Type>	ValueNodeType;

	  public:
		/**
		 * Create a property with singular path.
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
		Property (PropertyDirectoryNode* root, std::string const& path);

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

		// BaseProperty API
		std::string
		stringify() const override;

		// BaseProperty API
		void
		parse (std::string const&) override;

		/**
		 * Return node casted to PropertyValueNode.
		 * If unable to cast, throw InvalidOperation.
		 * If property node doesn't exist, return nullptr.
		 */
		ValueNodeType*
		get_value_node() const;

		/**
		 * Return node casted to PropertyValueNode.
		 * If unable to cast, throw InvalidOperation.
		 * If property node doesn't exist, throw PropertyNotFound.
		 */
		ValueNodeType*
		get_value_node_signalling() const;

	  private:
		/**
		 * Ensure that the property exists in the tree.
		 */
		template<class V>
			ValueNodeType*
			ensure_path (std::string const& path, V value);
	};


inline
PropertyNotFound::PropertyNotFound (std::string const& message):
	Exception (message)
{ }


inline
SingularProperty::SingularProperty (std::string const& message):
	Exception (message)
{ }


inline
InvalidOperation::InvalidOperation (std::string const& message):
	Exception (message)
{ }


inline
BaseProperty::BaseProperty (PropertyDirectoryNode* root, std::string path):
	_root (root),
	_path (normalized_path (path))
{ }


inline
BaseProperty::~BaseProperty()
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
		{
			BasePropertyValueNode* val_node = dynamic_cast<BasePropertyValueNode*> (node);
			if (val_node)
				return val_node->is_nil();
			else
				throw InvalidOperation ("can't check if directory node is nil: " + _path);
		}
		else
			return true;
	}
	else
		throw SingularProperty ("can't read from a singular property: " + _path);
}


inline void
BaseProperty::set_nil()
{
	if (_root)
	{
		PropertyNode* node = get_node();
		if (node)
		{
			BasePropertyValueNode* val_node = dynamic_cast<BasePropertyValueNode*> (node);
			if (val_node)
				val_node->set_nil();
			else
				throw InvalidOperation ("can't set directory node to nil: " + _path);
		}
	}
	else
		throw SingularProperty ("can't write to a singular property: " + _path);
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


inline void
BaseProperty::set_path (std::string const& new_path)
{
	_path = normalized_path (new_path);
	// The node will be localized again, when it's needed:
	_node = nullptr;
}


inline void
BaseProperty::ensure_existence()
{
	if (is_nil())
		set_nil();
}


inline PropertyNode*
BaseProperty::get_node() const
{
	if (_root && !_path.empty())
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
	Property<T>::Property():
		Property ("")
	{ }


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
		Property (PropertyStorage::default_storage()->root(), path)
	{
		if (!_root)
			throw SingularProperty ("PropertyStorage is not initialized, can't construct Property with default storage: " + _path);
	}


template<class T>
	inline
	Property<T>::Property (PropertyDirectoryNode* node, std::string const& path):
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
			ValueNodeType* node = get_value_node();
			if (!node)
				return default_value;
			else
				return node->read (default_value);
		}
		return default_value;
	}


template<class T>
	inline typename
	Property<T>::Type
	Property<T>::read_signalling() const
	{
		if (_root)
			get_value_node_signalling()->read();
		else
			throw SingularProperty ("can't read from a singular property: " + _path);
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
			if (!_path.empty())
			{
				try {
					get_value_node_signalling()->write (value);
				}
				catch (PropertyNotFound)
				{
					ensure_path (_path, value);
				}
			}
		}
		else
			throw SingularProperty ("can't write to a singular property: " + _path);
	}


template<class T>
	inline void
	Property<T>::write_signalling (Type const& value)
	{
		if (_root)
			get_value_node_signalling()->write (value);
		else
			throw SingularProperty ("can't write to a singular property: " + _path);
	}


template<class T>
	inline void
	Property<T>::copy (Property<T> const& other)
	{
		write (other.read());
		if (other.is_nil())
			set_nil();
	}


template<class T>
	inline std::string
	Property<T>::stringify() const
	{
		if (_root)
			return get_value_node_signalling()->stringify();
		return "";
	}


template<class T>
	inline void
	Property<T>::parse (std::string const& value)
	{
		if (_root)
		{
			if (!_path.empty())
			{
				try {
					get_value_node_signalling()->parse (value);
				}
				catch (PropertyNotFound)
				{
					ValueNodeType* val_node = ensure_path (_path, Type());
					val_node->parse (value);
				}
			}
		}
		else
			throw SingularProperty ("can't write to a singular property: " + _path);
	}


template<class T>
	inline typename Property<T>::ValueNodeType*
	Property<T>::get_value_node() const
	{
		PropertyNode* node = get_node();
		if (node)
		{
			ValueNodeType* val_node = dynamic_cast<ValueNodeType*> (node);
			if (val_node)
				return val_node;
			else
				throw InvalidOperation ("incompatible type: " + _path);
		}
		else
			return nullptr;
	}


template<class T>
	inline typename Property<T>::ValueNodeType*
	Property<T>::get_value_node_signalling() const
	{
		ValueNodeType* val_node = get_value_node();
		if (val_node)
			return val_node;
		else
			throw PropertyNotFound ("could not find property by path: " + _path);
	}


template<class T>
	template<class V>
		inline typename Property<T>::ValueNodeType*
		Property<T>::ensure_path (std::string const& path, V value)
		{
			std::string::size_type s = path.find_last_of ('/');
			std::string dir = path.substr (0, s);
			std::string pro = path.substr (s + 1);

			PropertyDirectoryNode* parent = _root;
			if (s != std::string::npos)
				parent = _root->mkpath (dir);
			ValueNodeType* child = new ValueNodeType (pro, value);
			parent->add_child (child);
			return child;
		}


/*
 * Shortcut types
 */


typedef Property<bool>			PropertyBoolean;
typedef Property<int64_t>		PropertyInteger;
typedef Property<double>		PropertyFloat;
typedef Property<std::string>	PropertyString;
typedef Property<Angle>			PropertyAngle;
typedef Property<Pressure>		PropertyPressure;
typedef Property<Frequency>		PropertyFrequency;
typedef Property<Length>		PropertyLength;
typedef Property<Time>			PropertyTime;
typedef Property<Speed>			PropertySpeed;

} // namespace Xefis

#endif

