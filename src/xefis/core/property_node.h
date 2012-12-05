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

typedef std::list<PropertyNode*> PropertyNodeList;


/**
 * Indicates invalid operation on node of a specific type,
 * for example attempt to read int value of a directory-type
 * node.
 */
class PropertyAccessError: public std::logic_error
{
  public:
	PropertyAccessError (const char* message);
};


/**
 * Indicates that there was a path conflict while creating
 * directory path with mkpath().
 */
class PropertyPathConflict: public std::runtime_error
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
	is_nil() const;

	/**
	 * Inverse of is_nil().
	 */
	bool
	valid() const;

	/**
	 * Write nil value to this property.
	 */
	void
	set_nil();

	/**
	 * Return node name.
	 */
	std::string
	name() const;

	/**
	 * Return node type.
	 */
	PropertyType
	type() const;

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
		write (tType value);

	/**
	 * Return parent node.
	 */
	PropertyNode*
	parent() const;

	/**
	 * Return root node. Traverse parents until root node.
	 * Return self if this node is the root node.
	 */
	PropertyNode*
	root();

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

  protected:
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

  private:
	PropertyNode*		_parent = nullptr;
	PropertyType		_type;
	std::string			_name;
	union {
		bool			_value_bool;
		int				_value_int;
		double			_value_double;
	};
	std::string			_value_string;
	bool				_is_nil = false;
	PropertyNodeList	_children;
	NameLookup			_children_by_name;
};


inline
PropertyAccessError::PropertyAccessError (const char* message):
	std::logic_error (message)
{ }


inline
PropertyPathConflict::PropertyPathConflict (const char* message):
	std::runtime_error (message)
{ }


inline
PropertyNode::PropertyNode (std::string const& name):
	_type (PropDirectory),
	_name (name)
{ }


inline
PropertyNode::PropertyNode (std::string const& name, bool value):
	_type (PropBoolean),
	_name (name),
	_value_bool (value)
{ }


inline
PropertyNode::PropertyNode (std::string const& name, int value):
	_type (PropInteger),
	_name (name),
	_value_int (value)
{ }


inline
PropertyNode::PropertyNode (std::string const& name, double value):
	_type (PropFloat),
	_name (name),
	_value_double (value)
{ }


inline
PropertyNode::PropertyNode (std::string const& name, const char* value):
	_type (PropString),
	_name (name),
	_value_string (value)
{ }


inline
PropertyNode::PropertyNode (std::string const& name, std::string const& value):
	_type (PropString),
	_name (name),
	_value_string (value)
{ }


template<>
	inline
	PropertyNode::PropertyNode<bool> (std::string const& name):
		_type (PropBoolean),
		_name (name),
		_value_bool(),
		_is_nil (true)
	{ }


template<>
	inline
	PropertyNode::PropertyNode<int> (std::string const& name):
		_type (PropInteger),
		_name (name),
		_value_int(),
		_is_nil (true)
	{ }


template<>
	inline
	PropertyNode::PropertyNode<double> (std::string const& name):
		_type (PropFloat),
		_name (name),
		_value_double(),
		_is_nil (true)
	{ }


template<>
	inline
	PropertyNode::PropertyNode<std::string> (std::string const& name):
		_type (PropString),
		_name (name),
		_value_string(),
		_is_nil (true)
	{ }


inline
PropertyNode::~PropertyNode()
{
	clear();
}


inline bool
PropertyNode::is_nil() const
{
	return _is_nil;
}


inline bool
PropertyNode::valid() const
{
	return !_is_nil;
}


inline void
PropertyNode::set_nil()
{
	_is_nil = true;
}


inline std::string
PropertyNode::name() const
{
	return _name;
}


inline PropertyType
PropertyNode::type() const
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
	PropertyNode::write (tType value)
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
				throw PropertyAccessError ("can't write from a directory property");
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
PropertyNode::parent() const
{
	return _parent;
}


inline PropertyNode*
PropertyNode::root()
{
	PropertyNode* p = this;
	while (p->_parent)
		p = p->_parent;
	return p;
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
				throw PropertyAccessError ("can't read from a directory property");
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
