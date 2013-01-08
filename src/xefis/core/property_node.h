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

#ifndef XEFIS__CORE__PROPERTY_NODE_H__INCLUDED
#define XEFIS__CORE__PROPERTY_NODE_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>
#include <string>
#include <list>
#include <map>
#include <memory>
#include <stdint.h>

// Boost:
#include <boost/lexical_cast.hpp>

// Xefis:
#include <xefis/config/all.h>
#include "property_storage.h"


namespace Xefis {

enum PropertyType
{
	PropDirectory,
	PropBoolean,
	PropInteger,
	PropFloat,
	PropString
};

class PropertyNode;
class PropertyStorage;

typedef std::list<PropertyNode*> PropertyNodeList;


/**
 * Indicates invalid operation on node of a specific type,
 * for example attempt to read int value of a directory-type
 * node.
 */
class PropertyAccessError: public Exception
{
  public:
	PropertyAccessError (const char* message);
};


/**
 * Indicates that there was a path conflict while creating
 * directory path with mkpath().
 */
class PropertyPathConflict: public Exception
{
  public:
	PropertyPathConflict (const char* message);
};


/**
 * A private API class.
 * The struct that holds property's data.
 */
class PropertyNode
{
	typedef std::map<std::string, PropertyNodeList::iterator> NameLookup;

	friend class PropertyStorage;

  private:
	/**
	 * Create a root node.
	 */
	PropertyNode (PropertyStorage*);

  public:
	/**
	 * Create a directory node.
	 */
	PropertyNode (std::string const& name);

	/**
	 * Create a bool node.
	 */
	PropertyNode (std::string const& name, bool value);

	/**
	 * Create an int node.
	 */
	PropertyNode (std::string const& name, int value);

	/**
	 * Create a double-precision float node.
	 */
	PropertyNode (std::string const& name, double value);

	/**
	 * Create a string node.
	 */
	PropertyNode (std::string const& name, const char* value);

	/**
	 * Create a string node.
	 */
	PropertyNode (std::string const& name, std::string const& value);

	/**
	 * Create a nil node of given type.
	 */
	template<class tType>
		PropertyNode (std::string const& name);

	/**
	 * Deletes child properties.
	 */
	~PropertyNode();

	/**
	 * Return true if property is nil.
	 */
	bool
	is_nil() const noexcept;

	/**
	 * Inverse of is_nil().
	 */
	bool
	valid() const noexcept;

	/**
	 * Write nil value to this property.
	 */
	void
	set_nil() noexcept;

	/**
	 * Return node name.
	 */
	std::string const&
	name() const noexcept;

	/**
	 * Return node path.
	 */
	std::string const&
	path() const noexcept;

	/**
	 * Return node type.
	 */
	PropertyType
	type() const noexcept;

	/**
	 * Return current value converted to the type tType,
	 * where tType can be bool, int, double, std::string (otherwise
	 * a compilation error occurs).
	 * If the node is a directory-type node or a null node,
	 * throw PropertyAccessError.
	 */
	template<class tType>
		tType
		read() const;

	/**
	 * Write new value to the node. The value will be converted
	 * to the node type.
	 * If the node is a directory-type node or a null node,
	 * throw PropertyAccessError.
	 */
	template<class tType>
		void
		write (tType const& value);

	/**
	 * Return parent node.
	 * Root node has no parent.
	 */
	PropertyNode*
	parent() const noexcept;

	/**
	 * Return root node. Traverse parents until root node.
	 * Return self if this node is the root node.
	 */
	PropertyNode*
	root() noexcept;

	/**
	 * Return list of child nodes.
	 */
	PropertyNodeList
	children() const;

	/**
	 * Find a child by its name.
	 * Return nullptr, if not found.
	 */
	PropertyNode*
	child (std::string const& name);

	/**
	 * Search for a property matching given path.
	 * "/" at the beginning jumps to the root node.
	 * "//" at any point jumps to the root node.
	 * ".." jumps to the parent node.
	 * Return nullptr if the node is not found.
	 * For accessing direct descendands, child() is faster.
	 */
	PropertyNode*
	locate (std::string const& path);

	/**
	 * Create directory hierarchy. Return bottom-leaf directory node.
	 * If there's already an existing node in the path,
	 * and it's not a directory-type node, throw PropertyPathConflict.
	 * The part already created will remain in there.
	 */
	PropertyNode*
	mkpath (std::string const& path);

	/**
	 * Add new property as a subproperty.
	 */
	PropertyNode*
	add_child (PropertyNode*);

	/**
	 * Remove child property.
	 * Do not delete the child.
	 */
	void
	remove_child (PropertyNode*);

	/**
	 * Removes and deletes child properties.
	 */
	void
	clear();

	/**
	 * Return pointer to the PropertyStorage object.
	 * If called on non-root property, it will take
	 * additional time to traverse to the root node
	 * to return its storage().
	 */
	PropertyStorage*
	storage() noexcept;

  private:
	/**
	 * Helper function for reading data.
	 */
	template<class tType>
		tType
		read_convertible() const;

	/**
	 * Helper function for writing data.
	 */
	template<class tType>
		void
		write_convertible (tType);

	/**
	 * Update self-cached location.
	 */
	void
	update_path();

  private:
	PropertyNode*		_parent			= nullptr;
	PropertyStorage*	_storage		= nullptr;
	PropertyType		_type;
	std::string			_name;
	std::string			_path;
	union {
		bool			_value_bool;
		int				_value_int;
		double			_value_double;
	};
	std::string			_value_string;
	bool				_is_nil			= false;
	PropertyNodeList	_children;
	NameLookup			_children_by_name;
};


inline
PropertyAccessError::PropertyAccessError (const char* message):
	Exception (message)
{ }


inline
PropertyPathConflict::PropertyPathConflict (const char* message):
	Exception (message)
{ }


inline
PropertyNode::PropertyNode (PropertyStorage* storage):
	PropertyNode ("") // It's a directory
{
	_storage = storage;
}


inline
PropertyNode::PropertyNode (std::string const& name):
	_type (PropDirectory),
	_name (name)
{
	update_path();
}


inline
PropertyNode::PropertyNode (std::string const& name, bool value):
	_type (PropBoolean),
	_name (name),
	_value_bool (value)
{
	update_path();
}


inline
PropertyNode::PropertyNode (std::string const& name, int value):
	_type (PropInteger),
	_name (name),
	_value_int (value)
{
	update_path();
}


inline
PropertyNode::PropertyNode (std::string const& name, double value):
	_type (PropFloat),
	_name (name),
	_value_double (value)
{
	update_path();
}


inline
PropertyNode::PropertyNode (std::string const& name, const char* value):
	_type (PropString),
	_name (name),
	_value_string (value)
{
	update_path();
}


inline
PropertyNode::PropertyNode (std::string const& name, std::string const& value):
	_type (PropString),
	_name (name),
	_value_string (value)
{
	update_path();
}


template<>
	inline
	PropertyNode::PropertyNode<bool> (std::string const& name):
		_type (PropBoolean),
		_name (name),
		_value_bool(),
		_is_nil (true)
	{
		update_path();
	}


template<>
	inline
	PropertyNode::PropertyNode<int> (std::string const& name):
		_type (PropInteger),
		_name (name),
		_value_int(),
		_is_nil (true)
	{
		update_path();
	}


template<>
	inline
	PropertyNode::PropertyNode<double> (std::string const& name):
		_type (PropFloat),
		_name (name),
		_value_double(),
		_is_nil (true)
	{
		update_path();
	}


template<>
	inline
	PropertyNode::PropertyNode<std::string> (std::string const& name):
		_type (PropString),
		_name (name),
		_value_string(),
		_is_nil (true)
	{
		update_path();
	}


inline
PropertyNode::~PropertyNode()
{
	clear();
}


inline bool
PropertyNode::is_nil() const noexcept
{
	return _is_nil;
}


inline bool
PropertyNode::valid() const noexcept
{
	return !_is_nil;
}


inline void
PropertyNode::set_nil() noexcept
{
	_is_nil = true;
}


inline std::string const&
PropertyNode::name() const noexcept
{
	return _name;
}


inline std::string const&
PropertyNode::path() const noexcept
{
	return _path;
}


inline PropertyType
PropertyNode::type() const noexcept
{
	return _type;
}


template<class tType>
	inline tType
	PropertyNode::read() const
	{
		return read_convertible<tType>();
	}


template<>
	inline std::string
	PropertyNode::read() const
	{
		switch (_type)
		{
			case PropDirectory:
				throw PropertyAccessError ("can't read from a directory property");
			case PropBoolean:
				return boost::lexical_cast<std::string> (_value_bool);
			case PropInteger:
				return boost::lexical_cast<std::string> (_value_int);
			case PropFloat:
				return boost::lexical_cast<std::string> (_value_double);
			case PropString:
				return _value_string;
			default:
				return std::string();
		}
	}


template<class tType>
	inline void
	PropertyNode::write (tType const& value)
	{
		write_convertible (value);
	}


template<>
	inline void
	PropertyNode::write (std::string const& value)
	{
		switch (_type)
		{
			case PropDirectory:
				throw PropertyAccessError ("can't write to a directory property");
			case PropBoolean:
				_value_bool = boost::lexical_cast<bool> (value);
				break;
			case PropInteger:
				_value_int = boost::lexical_cast<int> (value);
				break;
			case PropFloat:
				_value_double = boost::lexical_cast<double> (value);
				break;
			case PropString:
				_value_string = value;
				break;
		}
		_is_nil = false;
	}


inline PropertyNode*
PropertyNode::parent() const noexcept
{
	return _parent;
}


inline PropertyNode*
PropertyNode::root() noexcept
{
	PropertyNode* p = this;
	while (p->_parent)
		p = p->_parent;
	return p;
}


inline PropertyNode*
PropertyNode::locate (std::string const& path)
{
	if (path.empty())
		return this;

	// If we are root node, try searching PropertyStorage cache first:
	if (!_parent && _storage)
	{
		PropertyNode* node = _storage->locate (path[0] == '/' ? path : '/' + path);
		if (node)
			return node;
		else
			// If not found by absolute path in Storage, then it doesn't exist:
			return nullptr;
	}

	if (path[0] == '/')
		return root()->locate (path.substr (1));

	std::string::size_type slash = path.find ('/');
	std::string segment = path.substr (0, slash);
	std::string rest;
	if (slash != std::string::npos)
		rest = path.substr (slash + 1);

	if (segment == ".")
		return locate (rest);
	else if (segment == "..")
	{
		if (_parent)
			return _parent->locate (rest);
		return nullptr;
	}
	else
	{
		PropertyNode* c = child (segment);
		if (c)
			return c->locate (rest);
		return nullptr;
	}
}


inline PropertyStorage*
PropertyNode::storage() noexcept
{
	return _storage ? _storage : root()->_storage;
}


template<class tType>
	inline tType
	PropertyNode::read_convertible() const
	{
		switch (_type)
		{
			case PropDirectory:
				throw PropertyAccessError ("property is a directory and doesn't have any value");
			case PropBoolean:
				return _value_bool;
			case PropInteger:
				return _value_int;
			case PropFloat:
				return _value_double;
			case PropString:
				return boost::lexical_cast<tType> (_value_string);
			default:
				return tType();
		}
	}


template<class tType>
	inline void
	PropertyNode::write_convertible (tType value)
	{
		switch (_type)
		{
			case PropDirectory:
				throw PropertyAccessError ("can't write to a directory property");
			case PropBoolean:
				_value_bool = value;
				break;
			case PropInteger:
				_value_int = value;
				break;
			case PropFloat:
				_value_double = value;
				break;
			case PropString:
				_value_string = boost::lexical_cast<std::string> (value);
				break;
		}
		_is_nil = false;
	}

} // namespace Xefis

#endif

