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

#ifndef XEFIS__CORE__PROPERTY_NODE_H__INCLUDED
#define XEFIS__CORE__PROPERTY_NODE_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>
#include <string>
#include <list>
#include <map>
#include <set>
#include <memory>
#include <type_traits>
#include <stdint.h>

// Boost:
#include <boost/lexical_cast.hpp>
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/property_storage.h>
#include <xefis/core/v1/property_utils.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/string.h>
#include <xefis/utility/blob.h>
#include <xefis/utility/time_helper.h>


namespace v1 {
using namespace xf;

// Forward declarations:
class PropertyDirectoryNode;
class PropertyStorage;
template<class T>
	class PropertyValueNode;

typedef std::list<PropertyNode*> PropertyNodeList;


/**
 * Property tree node.
 */
class PropertyNode: private Noncopyable
{
	friend class PropertyStorage;
	friend class PropertyDirectoryNode;

  public:
	// Used to tell if node value has changed:
	typedef uint64_t Serial;

  protected:
	/**
	 * Create a root node.
	 */
	PropertyNode (PropertyStorage*);

	// Ctor
	PropertyNode (std::string const& name);

  public:
	// Dtor
	virtual
	~PropertyNode();

	/**
	 * Return node name.
	 */
	std::string const&
	name() const noexcept;

	/**
	 * Return node path.
	 */
	PropertyPath const&
	path() const noexcept;

	/**
	 * Return parent node.
	 * Root node has no parent.
	 */
	PropertyDirectoryNode*
	parent() const noexcept;

	/**
	 * Return root node. Traverse parents until root node.
	 * Return self if this node is the root node.
	 */
	PropertyDirectoryNode*
	root() noexcept;

	/**
	 * Return pointer to the PropertyStorage object.
	 * If called on non-root property, it will take
	 * additional time to traverse to the root node
	 * to return its storage().
	 */
	PropertyStorage*
	storage() noexcept;

	/**
	 * Return node serial value.
	 * It's incremented every time node value
	 * is changed.
	 */
	Serial
	serial() const noexcept;

  protected:
	/**
	 * Increment the serial value.
	 */
	void
	bump_serial() noexcept;

  private:
	/**
	 * Update self-cached location.
	 */
	void
	update_path();

  protected:
	PropertyDirectoryNode*	_parent		= nullptr;
	PropertyStorage*		_storage	= nullptr;
	std::string				_name;
	PropertyPath			_path;
	Serial					_serial		= 0;
};


/**
 * PropertyNode that is a directory and can
 * have children nodes.
 */
class PropertyDirectoryNode: public PropertyNode
{
	friend class PropertyStorage;

	typedef std::map<std::string, PropertyNodeList::iterator> NameLookup;

  private:
	// Ctor
	PropertyDirectoryNode (PropertyStorage*);

  public:
	// Ctor
	PropertyDirectoryNode (std::string const& name);

	/**
	 * Deletes child properties.
	 */
	~PropertyDirectoryNode();

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
	locate (PropertyPath const& path);

	/**
	 * Create directory hierarchy. Return bottom-leaf directory node.
	 * If there's already an existing node in the path,
	 * and it's not a directory-type node, throw PropertyPathConflict.
	 * The part already created will remain in there.
	 */
	PropertyDirectoryNode*
	mkpath (PropertyPath const& path);

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

  private:
	PropertyNodeList	_children;
	NameLookup			_children_by_name;
};


/**
 * Non-template base for PropertyValueNode.
 */
class TypedPropertyValueNode: public PropertyNode
{
	template<class T>
		friend class PropertyValueNode;

  public:
	TypedPropertyValueNode (std::string const& name);

	/**
	 * Return timestamp of the value (time when it was modified).
	 * It's updated even if the same value was written as before.
	 */
	Time
	modification_timestamp() const noexcept;

	/**
	 * Return timestamp of the last non-nil value.
	 * It's updated even if the same value was written as before.
	 */
	Time
	valid_timestamp() const noexcept;

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
	 * Return human-readable value for UI.
	 */
	virtual std::string
	stringify() const = 0;

	/**
	 * Return binary blob representing value.
	 */
	virtual Blob
	binarify() const = 0;

	/**
	 * Return float-like value for the property.
	 */
	virtual double
	to_float (std::string const& unit) const = 0;

	/**
	 * Parse value and unit.
	 */
	virtual void
	parse (std::string const&) = 0;

	/**
	 * Parse value from binary representation.
	 */
	virtual void
	parse (Blob const&) = 0;

  private:
	bool	_is_nil					= false;
	Time	_modification_timestamp	= 0_s;
	Time	_valid_timestamp		= 0_s;
};


/**
 * PropertyNode that holds a value.
 */
template<class tType>
	class PropertyValueNode: public TypedPropertyValueNode
	{
	  public:
		typedef tType Type;

	  public:
		// Ctor
		PropertyValueNode (std::string const& name, Type value);

		/**
		 * Return stored value. If node is a nil-node,
		 * throw NilNode exception.
		 */
		Type const&
		read() const;

		/**
		 * Return stored value. If node is a nil-node,
		 * return default_value.
		 */
		Type
		read (Type default_value) const;

		/**
		 * Write value to this node.
		 */
		void
		write (Type const& value);

		/**
		 * Write value to this node. If Optional value
		 * doesn't hold any value, set node to nil.
		 */
		void
		write (std::optional<Type> const& value);

		/**
		 * Return human-readable value for UI.
		 */
		std::string
		stringify() const override;

		/**
		 * Return binary blob representing value.
		 */
		Blob
		binarify() const override;

		/**
		 * Return float-like value.
		 */
		double
		to_float (std::string const& unit) const override;

		/**
		 * Parse value and unit.
		 */
		void
		parse (std::string const&) override;

		/**
		 * Parse value from binary representation.
		 */
		void
		parse (Blob const&) override;

	  private:
		Type _value;
	};


inline
PropertyNode::PropertyNode (PropertyStorage* storage)
{
	_storage = storage;
}


inline
PropertyNode::PropertyNode (std::string const& name):
	_name (name)
{ }


inline
PropertyNode::~PropertyNode()
{ }


inline std::string const&
PropertyNode::name() const noexcept
{
	return _name;
}


inline PropertyPath const&
PropertyNode::path() const noexcept
{
	return _path;
}


inline PropertyDirectoryNode*
PropertyNode::parent() const noexcept
{
	return _parent;
}


inline PropertyDirectoryNode*
PropertyNode::root() noexcept
{
	PropertyNode* p = this;
	while (p->_parent)
		p = p->_parent;
	return dynamic_cast<PropertyDirectoryNode*> (p);
}


inline PropertyStorage*
PropertyNode::storage() noexcept
{
	return _storage ? _storage : root()->_storage;
}


inline PropertyNode::Serial
PropertyNode::serial() const noexcept
{
	return _serial;
}


inline void
PropertyNode::bump_serial() noexcept
{
	++_serial;
}


inline
PropertyDirectoryNode::PropertyDirectoryNode (PropertyStorage* storage):
	PropertyNode (storage)
{ }


inline
PropertyDirectoryNode::PropertyDirectoryNode (std::string const& name):
	PropertyNode (name)
{ }


inline
PropertyDirectoryNode::~PropertyDirectoryNode()
{
	clear();
}


inline
TypedPropertyValueNode::TypedPropertyValueNode (std::string const& name):
	PropertyNode (name)
{ }


inline Time
TypedPropertyValueNode::modification_timestamp() const noexcept
{
	return _modification_timestamp;
}


inline Time
TypedPropertyValueNode::valid_timestamp() const noexcept
{
	return _valid_timestamp;
}


inline bool
TypedPropertyValueNode::is_nil() const noexcept
{
	return _is_nil;
}


inline bool
TypedPropertyValueNode::valid() const noexcept
{
	return !_is_nil;
}


inline void
TypedPropertyValueNode::set_nil() noexcept
{
	_modification_timestamp = TimeHelper::now();

	if (!_is_nil)
	{
		_is_nil = true;
		bump_serial();
	}
}


template<class T>
	inline
	PropertyValueNode<T>::PropertyValueNode (std::string const& name, Type value):
		TypedPropertyValueNode (name),
		_value (value)
	{ }


template<class T>
	inline typename PropertyValueNode<T>::Type const&
	PropertyValueNode<T>::read() const
	{
		if (_is_nil)
			throw NilNode();
		else
			return _value;
	}


template<class T>
	inline typename PropertyValueNode<T>::Type
	PropertyValueNode<T>::read (Type default_value) const
	{
		if (_is_nil)
			return default_value;
		else
			return _value;
	}


template<class T>
	inline void
	PropertyValueNode<T>::write (Type const& value)
	{
		_modification_timestamp = TimeHelper::now();
		_valid_timestamp = _modification_timestamp;

		if (_is_nil || _value != value)
		{
			_value = value;
			_is_nil = false;
			bump_serial();
		}
	}


template<class T>
	inline void
	PropertyValueNode<T>::write (std::optional<Type> const& value)
	{
		if (value)
			write (*value);
		else
			set_nil();
	}


template<class T>
	inline std::string
	PropertyValueNode<T>::stringify() const
	{
		using std::to_string;
		return to_string (_value);
	}


template<>
	inline std::string
	PropertyValueNode<bool>::stringify() const
	{
		return _value ? "true" : "false";
	}


template<>
	inline std::string
	PropertyValueNode<std::string>::stringify() const
	{
		return _value;
	}


template<class T>
	inline Blob
	PropertyValueNode<T>::binarify() const
	{
		if (!_is_nil)
			return si::to_blob (_value);
		else
			return Blob();
	}


template<>
	inline Blob
	PropertyValueNode<bool>::binarify() const
	{
		if (!_is_nil)
			return _value ? Blob ({ 0x01 }) : Blob ({ 0x00 });
		else
			return Blob();
	}


template<>
	inline Blob
	PropertyValueNode<int64_t>::binarify() const
	{
		if (!_is_nil)
		{
			int64_t int_value = _value;
			boost::endian::native_to_little (int_value);
			return make_blob (&int_value, sizeof (int_value));
		}
		else
			return Blob();
	}


template<>
	inline Blob
	PropertyValueNode<double>::binarify() const
	{
		if (!_is_nil)
		{
			double double_value = _value;
			boost::endian::native_to_little (double_value);
			return make_blob (&double_value, sizeof (double_value));
		}
		else
			return Blob();
	}


template<>
	inline Blob
	PropertyValueNode<std::string>::binarify() const
	{
		if (!_is_nil)
		{
			Blob result ({ 0x00 });
			result.insert (result.end(), _value.begin(), _value.end());
			return result;
		}
		else
			return Blob();
	}


template<class T>
	inline double
	PropertyValueNode<T>::to_float (std::string const& unit) const
	{
		return ::quantity (_value, unit);
	}


template<>
	inline double
	PropertyValueNode<std::string>::to_float (std::string const&) const
	{
		return 0.0;
	}


template<class T>
	inline void
	PropertyValueNode<T>::parse (std::string const& str)
	{
		write (si::parse<Type> (str));
	}


template<>
	inline void
	PropertyValueNode<bool>::parse (std::string const& str)
	{
		write (str == "true" || str == "1");
	}


template<>
	inline void
	PropertyValueNode<int64_t>::parse (std::string const& str)
	{
		try {
			write (boost::lexical_cast<int64_t> (str));
		}
		catch (boost::bad_lexical_cast&)
		{
			throw si::UnparsableValue ("error while parsing: " + str);
		}
	}


template<>
	inline void
	PropertyValueNode<double>::parse (std::string const& str)
	{
		try {
			write (boost::lexical_cast<double> (str));
		}
		catch (boost::bad_lexical_cast&)
		{
			throw si::UnparsableValue ("error while parsing: " + str);
		}
	}


template<>
	inline void
	PropertyValueNode<std::string>::parse (std::string const& str)
	{
		write (str);
	}


template<class T>
	inline void
	PropertyValueNode<T>::parse (Blob const& blob)
	{
		if (blob.empty())
			set_nil();
		else
			write (si::parse<Type> (blob));
	}


template<>
	inline void
	PropertyValueNode<bool>::parse (Blob const& blob)
	{
		if (blob.empty())
			set_nil();
		else
			write (blob[0] != 0x00);
	}


template<>
	inline void
	PropertyValueNode<int64_t>::parse (Blob const& blob)
	{
		if (blob.empty())
			set_nil();
		else if (blob.size() == sizeof (int64_t))
		{
			int64_t int_value = *reinterpret_cast<int64_t const*> (blob.data());
			boost::endian::little_to_native (int_value);
			write (int_value);
		}
	}


template<>
	inline void
	PropertyValueNode<double>::parse (Blob const& blob)
	{
		if (blob.empty())
			set_nil();
		else if (blob.size() == sizeof (double))
		{
			double double_value = *reinterpret_cast<double const*> (blob.data());
			boost::endian::little_to_native (double_value);
			write (double_value);
		}
	}


template<>
	inline void
	PropertyValueNode<std::string>::parse (Blob const& blob)
	{
		if (blob.empty())
			set_nil();
		else if (blob[0] == 0x00)
			write (std::string (blob.begin() + 1, blob.end()));
	}

} // namespace v1

#endif

