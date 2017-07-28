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

#ifndef XEFIS__CORE__PROPERTY_H__INCLUDED
#define XEFIS__CORE__PROPERTY_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>
#include <string>
#include <limits>

// Boost:
#include <boost/optional.hpp>
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/property_node.h>
#include <xefis/core/v1/property_storage.h>
#include <xefis/core/v1/property_utils.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/time_helper.h>


namespace v1 {
using namespace xf;

class GenericProperty
{
  public:
	// Ctor
	GenericProperty();

	// Ctor
	GenericProperty (GenericProperty const&) = default;

	// Assignment
	GenericProperty&
	operator= (GenericProperty const&) = default;

	// Ctor
	GenericProperty (PropertyPath const& path);

	// Ctor
	GenericProperty (PropertyDirectoryNode* root, PropertyPath const& path);

	// Dtor
	virtual
	~GenericProperty();

	/**
	 * Return timestamp of the value (time when it was modified).
	 * It's updated even if the same value was written as before.
	 */
	Time
	modification_timestamp() const;

	/**
	 * Return timestamp of the last non-nil value.
	 * It's updated even if the same value was written as before.
	 */
	Time
	valid_timestamp() const;

	/**
	 * Return age of the value (time since it was last modified).
	 */
	Time
	modification_age() const;

	/**
	 * Return age of the non-nil value (time since it was last set to a non-nil value).
	 */
	Time
	valid_age() const;

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
	 * Configured means not singular and having path != "".
	 */
	bool
	configured() const noexcept;

	/**
	 * Valid means not singular and not nil.
	 */
	bool
	valid() const noexcept;

	/**
	 * Return property path.
	 */
	PropertyPath const&
	path() const noexcept;

	/**
	 * Point this property to another PropertyNode.
	 */
	void
	set_path (PropertyPath const&);

	/**
	 * Return the serial value of the property.
	 */
	PropertyNode::Serial
	serial() const;

	/**
	 * Return true if the PropertyNode value has changed
	 * since last read().
	 */
	bool
	fresh() const;

	/**
	 * Convenience method that checks for valid()ity and fresh()ness.
	 */
	bool
	valid_and_fresh() const;

	/**
	 * Check whether the node this property points to,
	 * is a PropertyValueNode of given Target type.
	 */
	template<class Target>
		bool
		is_type() const;

	/**
	 * Return proper node. If cached node's path
	 * matches this property's path, return it.
	 * Otherwise, locate it.
	 */
	PropertyNode*
	get_node() const;

	/**
	 * Ensures that this property exists.
	 */
	virtual void
	ensure_existence()
	{ }

	/**
	 * Ensures that this property exists.
	 *
	 * \param	type
	 *			Name of a type. See lib/si for type names.
	 */
	void
	ensure_existence (PropertyType const& type);

	/**
	 * Set value from humanized string (eg. "10 kt").
	 * This version (not overridden) doesn't create a nil node,
	 * when it can't find one, but quietly does nothing.
	 *
	 * TODO parse with unit, get info about unit (DynamicUnit/name),
	 * use ensure_existence (std::string type) to create node and
	 * parse.
	 */
	virtual void
	parse_existing (std::string const&);

	/**
	 * Set value from binary blob.
	 * This version (not overridden) doesn't create a nil node,
	 * when it can't find one, but quietly does nothing.
	 *
	 * \param	type
	 *			Name of a type. See lib/si for type names.
	 */
	virtual void
	parse_existing (Blob const&, PropertyType const& type);

	/**
	 * Return humanized value (eg. value with unit).
	 */
	virtual std::string
	stringify() const;

	/**
	 * Return humanized value (using boost::format and specified unit).
	 */
	virtual std::string
	stringify (boost::format, std::string const& unit, std::string value_if_nil) const;

	/**
	 * Return binary representation of the value.
	 */
	virtual Blob
	binarify() const;

	/**
	 * Return float-like value of the property.
	 */
	virtual double
	to_float (std::string unit) const;

	/**
	 * Create new property node of given type.
	 * \throw	TypeConflict
	 * 			when there's another property under the same path with different type.
	 * \throw	BadType
	 * 			when type doesn't name correct type.
	 */
	static void
	create (PropertyPath const& path, PropertyType const& type);

	/**
	 * Create (if doesn't exist) property and
	 * set to given argument.
	 */
	virtual void
	create_and_parse (std::string const&);

	/**
	 * Create (if doesn't exist) property and
	 * set to given argument.
	 */
	virtual void
	create_and_parse (Blob const&);

  protected:
	/**
	 * Reset flag that the property is fresh().
	 */
	void
	unfresh() const;

	/**
	 * Normalize path so if there's "//" in it,
	 * it will be replaced by leading "/".
	 */
	static PropertyPath
	normalized_path (PropertyPath path);

  protected:
	PropertyDirectoryNode*			_root = nullptr;
	mutable PropertyNode*			_node = nullptr;
	PropertyPath					_path;
	mutable PropertyNode::Serial	_last_read_serial = 0;
};


/**
 * A property reference. Doesn't hold the data, but only the path,
 * and queries property storage whenever needed.
 */
template<class tType>
	class Property: public GenericProperty
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
		 * Create a Property that belongs to xf::PropertyStorage::root()
		 * (the default storage) and is bound to given path.
		 * NOTE: The PropertyStorage must be initialized before attempting
		 * to use this constructor.
		 */
		Property (PropertyPath const& path);

		/**
		 * Create a Property that belongs to a PropertyStorage,
		 * bound to given path.
		 */
		Property (PropertyDirectoryNode* root, PropertyPath const& path);

		/**
		 * Copy from other property.
		 */
		Property&
		operator= (Property const& other);

		/**
		 * Get std::optional value. Takes 'nil' value into account.
		 */
		std::optional<Type>
		get_optional() const;

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
		 * Pointer operator for accessing members of the stored value.
		 * \throw	SingularProperty if property is singular.
		 * \throw	NilNode if node is a nil-node.
		 */
		Type const*
		operator->() const;

		/**
		 * Overload for write (std::optional<Type>&);
		 * \return	*this
		 */
		Property<Type>&
		operator= (std::optional<Type> const&);

		/**
		 * Write to the property.
		 * If std::optional doesn't hold a value, set the property to nil.
		 */
		void
		write (std::optional<Type> const&);

		/**
		 * Write to the property.
		 * If std::optional doesn't hold a value, set the property to nil.
		 * If node can't be found, throw PropertyNotFound.
		 */
		void
		write_signalling (std::optional<Type> const&);

		/**
		 * Sets value (like write) if property is not singular
		 * and if it's nil. Otherwise it's a no-op.
		 */
		void
		set_default (Type const&);

		/**
		 * Copy other node.
		 */
		void
		copy_from (Property const& from);

		// GenericProperty API
		void
		ensure_existence();

		// GenericProperty API
		void
		create_and_parse (std::string const&) override;

		// GenericProperty API
		void
		create_and_parse (Blob const&) override;

		/**
		 * Return node casted to PropertyValueNode.
		 * If unable to cast, throw TypeConflict.
		 * If property node doesn't exist, return nullptr.
		 */
		ValueNodeType*
		get_value_node() const;

		/**
		 * Return node casted to PropertyValueNode.
		 * If unable to cast, throw TypeConflict.
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
			ensure_path (PropertyPath const& path, V value);
	};


inline
GenericProperty::GenericProperty():
	_root (PropertyStorage::default_storage()->root())
{ }


inline
GenericProperty::GenericProperty (PropertyPath const& path):
	_root (PropertyStorage::default_storage()->root()),
	_path (normalized_path (path))
{ }


inline
GenericProperty::GenericProperty (PropertyDirectoryNode* root, PropertyPath const& path):
	_root (root),
	_path (normalized_path (path))
{ }


inline
GenericProperty::~GenericProperty()
{ }


inline Time
GenericProperty::modification_timestamp() const
{
	if (_root)
	{
		PropertyNode* node = get_node();
		if (node)
		{
			TypedPropertyValueNode* val_node = dynamic_cast<TypedPropertyValueNode*> (node);
			if (val_node)
				return val_node->modification_timestamp();
			else
				throw InvalidOperation ("can't check timestamps on non-value node: " + _path.string());
		}
		else
			return 0_s;
	}
	else
		throw SingularProperty ("can't access node from a singular property: " + _path.string());
}


inline Time
GenericProperty::valid_timestamp() const
{
	if (_root)
	{
		PropertyNode* node = get_node();
		if (node)
		{
			TypedPropertyValueNode* val_node = dynamic_cast<TypedPropertyValueNode*> (node);
			if (val_node)
				return val_node->valid_timestamp();
			else
				throw InvalidOperation ("can't check timestamps on non-value node: " + _path.string());
		}
		else
			return 0_s;
	}
	else
		throw SingularProperty ("can't access node from a singular property: " + _path.string());
}


inline Time
GenericProperty::modification_age() const
{
	return TimeHelper::now() - modification_timestamp();
}


inline Time
GenericProperty::valid_age() const
{
	return TimeHelper::now() - valid_timestamp();
}


inline bool
GenericProperty::is_nil() const
{
	if (_root)
	{
		PropertyNode* node = get_node();
		if (node)
		{
			TypedPropertyValueNode* val_node = dynamic_cast<TypedPropertyValueNode*> (node);
			if (val_node)
				return val_node->is_nil();
			else
				throw InvalidOperation ("can't check if directory node is nil: " + _path.string());
		}
		else
			return true;
	}
	else
		throw SingularProperty ("can't read from a singular property: " + _path.string());
}


inline void
GenericProperty::set_nil()
{
	if (_root)
	{
		PropertyNode* node = get_node();
		if (node)
		{
			TypedPropertyValueNode* val_node = dynamic_cast<TypedPropertyValueNode*> (node);
			if (val_node)
				val_node->set_nil();
			else
				throw InvalidOperation ("can't set directory node to nil: " + _path.string());
		}
	}
	else
		throw SingularProperty ("can't write to a singular property: " + _path.string());
}


inline bool
GenericProperty::is_singular() const noexcept
{
	return !_root;
}


inline bool
GenericProperty::configured() const noexcept
{
	return !is_singular() && !path().string().empty();
}


inline bool
GenericProperty::valid() const noexcept
{
	return !is_singular() && !is_nil();
}


inline PropertyPath const&
GenericProperty::path() const noexcept
{
	return _path;
}


inline void
GenericProperty::set_path (PropertyPath const& new_path)
{
	_path = normalized_path (new_path);
	// The node will be localized again, when it's needed:
	_node = nullptr;
}


inline PropertyNode::Serial
GenericProperty::serial() const
{
	PropertyNode* node = get_node();
	if (node)
		return node->serial();
	// If node becomes unavailable (not valid, singular or so),
	// return -1u.
	return std::numeric_limits<PropertyNode::Serial>::max();
}


inline bool
GenericProperty::fresh() const
{
	return serial() + 1 > _last_read_serial;
}


inline bool
GenericProperty::valid_and_fresh() const
{
	return valid() && fresh();
}


template<class Target>
	inline bool
	GenericProperty::is_type() const
	{
		PropertyNode const* node = get_node();
		if (!node)
			return false;
		return !!dynamic_cast<PropertyValueNode<Target> const*> (node);
	}


inline PropertyNode*
GenericProperty::get_node() const
{
	if (_root && !_path.string().empty())
	{
		if (_node && _node->path() == _path)
			return _node;

		// Recache:
		return _node = _root->locate (_path);
	}
	else
		return nullptr;
}


inline void
GenericProperty::ensure_existence (PropertyType const& type)
{
	create (path(), type);
}


inline void
GenericProperty::parse_existing (std::string const& str_value)
{
	if (_root)
	{
		if (!_path.string().empty())
		{
			PropertyNode* node = get_node();
			if (node)
			{
				TypedPropertyValueNode* typed_node = dynamic_cast<TypedPropertyValueNode*> (node);
				if (typed_node)
					typed_node->parse (str_value);
			}
			else
				throw PropertyNotFound ("could not set non-existing property");
		}
	}
	else
		throw SingularProperty ("can't write to a singular property: " + _path.string());
}


inline void
GenericProperty::parse_existing (Blob const& value, PropertyType const& /*type*/)
{
	// TODO ensure @type matches actual type of the node. Otherwise throw.
	if (_root)
	{
		if (!_path.string().empty())
		{
			PropertyNode* node = get_node();
			if (node)
			{
				TypedPropertyValueNode* typed_node = dynamic_cast<TypedPropertyValueNode*> (node);
				if (typed_node)
					typed_node->parse (value);
			}
			else
				throw PropertyNotFound ("could not set non-existing property");
		}
	}
	else
		throw SingularProperty ("can't write to a singular property: " + _path.string());
}


inline std::string
GenericProperty::stringify() const
{
	TypedPropertyValueNode* node = dynamic_cast<TypedPropertyValueNode*> (get_node());
	if (node)
		return node->stringify();
	return "";
}


inline std::string
GenericProperty::stringify (boost::format format, std::string const& unit, std::string value_if_nil) const
{
	try {
		if (is_nil())
			return value_if_nil;
		else if (is_type<std::string>())
			return (format % stringify()).str();
		else if (is_type<bool>())
			return (stringify() == "true") ? "ON" : "OFF";
		else
			return (format % to_float (unit)).str();
	}
	catch (UnsupportedUnit&)
	{
		throw StringifyError ("unit error");
	}
	catch (boost::io::too_few_args&)
	{
		throw StringifyError ("format: too few args");
	}
	catch (boost::io::too_many_args&)
	{
		throw StringifyError ("format: too many args");
	}
	catch (...)
	{
		throw StringifyError ("format error");
	}
}


inline Blob
GenericProperty::binarify() const
{
	TypedPropertyValueNode* node = dynamic_cast<TypedPropertyValueNode*> (get_node());
	if (node)
		return node->binarify();
	return Blob();
}


inline double
GenericProperty::to_float (std::string unit) const
{
	TypedPropertyValueNode* node = dynamic_cast<TypedPropertyValueNode*> (get_node());
	if (node)
		return node->to_float (unit);
	return 0.0;
}


inline void
GenericProperty::create_and_parse (std::string const&)
{
	throw InvalidCall ("GenericProperty::create_and_parse() can't be called directly");
}


inline void
GenericProperty::create_and_parse (Blob const&)
{
	throw InvalidCall ("GenericProperty::create_and_parse(Blob) can't be called directly");
}


inline void
GenericProperty::unfresh() const
{
	_last_read_serial = serial() + 1;
}


inline PropertyPath
GenericProperty::normalized_path (PropertyPath path)
{
	std::size_t p = path.string().rfind ("//");
	if (p != std::string::npos)
		return PropertyPath (path.string().substr (p + 1));
	return path;
}


template<class T>
	inline
	Property<T>::Property():
		Property (PropertyPath (""))
	{ }


template<class T>
	inline
	Property<T>::Property (Property const& other):
		GenericProperty (other)
	{ }


template<class T>
	inline
	Property<T>::Property (PropertyPath const& path):
		Property (PropertyStorage::default_storage()->root(), path)
	{
		if (!_root)
			throw SingularProperty ("PropertyStorage is not initialized, can't construct Property with default storage: " + _path.string());
	}


template<class T>
	inline
	Property<T>::Property (PropertyDirectoryNode* node, PropertyPath const& path):
		GenericProperty (node->root(), path)
	{ }


template<class T>
	inline Property<T>&
	Property<T>::operator= (Property const& other)
	{
		GenericProperty::operator= (other);
		return *this;
	}


template<class T>
	inline void
	Property<T>::ensure_existence()
	{
		if (is_nil())
		{
			write (Type());
			set_nil();
		}
	}


template<class T>
	inline std::optional<T>
	Property<T>::get_optional() const
	{
		if (is_nil())
			return std::optional<T>();
		return std::optional<T> (**this);
	}


template<class T>
	inline typename Property<T>::Type
	Property<T>::read (Type default_value) const
	{
		if (_root)
		{
			ValueNodeType* node = get_value_node();
			if (!node)
				return default_value;
			else
			{
				unfresh();
				return node->read (default_value);
			}
		}
		return default_value;
	}


template<class T>
	inline typename Property<T>::Type
	Property<T>::read_signalling() const
	{
		if (_root)
			return get_value_node_signalling()->read();
		else
			throw SingularProperty ("can't read from a singular property: " + _path.string());
	}


template<class T>
	inline typename Property<T>::Type
	Property<T>::operator*() const
	{
		return read();
	}


template<class T>
	inline typename Property<T>::Type const*
	Property<T>::operator->() const
	{
		if (_root)
			return &get_value_node_signalling()->read();
		else
			throw SingularProperty ("can't read from a singular property: " + _path.string());
	}


template<class T>
	inline Property<T>&
	Property<T>::operator= (std::optional<Type> const& value)
	{
		write (value);
		return *this;
	}


template<class T>
	inline void
	Property<T>::write (std::optional<Type> const& value)
	{
		if (value)
		{
			if (_root)
			{
				if (!_path.string().empty())
				{
					try {
						get_value_node_signalling()->write (*value);
					}
					catch (PropertyNotFound)
					{
						ensure_path (_path, *value);
					}
				}
			}
			else
				throw SingularProperty ("can't write to a singular property: " + _path.string());
		}
		else
			set_nil();
	}


template<class T>
	inline void
	Property<T>::write_signalling (std::optional<Type> const& value)
	{
		if (value)
		{
			if (_root)
				get_value_node_signalling()->write (*value);
			else
				throw SingularProperty ("can't write to a singular property: " + _path.string());
		}
		else
			set_nil();
	}


template<class T>
	inline void
	Property<T>::set_default (Type const& value)
	{
		if (!is_singular() && is_nil())
			write (value);
	}


template<class T>
	inline void
	Property<T>::copy_from (Property<T> const& from)
	{
		write (from.get_optional());
	}


template<class T>
	inline void
	Property<T>::create_and_parse (std::string const& value)
	{
		if (_root)
		{
			if (!_path.string().empty())
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
			throw SingularProperty ("can't write to a singular property: " + _path.string());
	}


template<class T>
	inline void
	Property<T>::create_and_parse (Blob const& value)
	{
		if (_root)
		{
			if (!_path.string().empty())
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
			throw SingularProperty ("can't write to a singular property: " + _path.string());
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
				throw TypeConflict (_path);
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
			throw PropertyNotFound ("could not find property by path: " + _path.string());
	}


template<class T>
	template<class V>
		inline typename Property<T>::ValueNodeType*
		Property<T>::ensure_path (PropertyPath const& path, V value)
		{
			std::string const& path_str = path.string();
			std::string::size_type s = path_str.find_last_of ('/');
			std::string dir = path_str.substr (0, s);
			std::string pro = path_str.substr (s + 1);

			PropertyDirectoryNode* parent = _root;
			if (s != std::string::npos)
				parent = _root->mkpath (PropertyPath (dir));
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
typedef Property<Acceleration>	PropertyAcceleration;
typedef Property<Angle>			PropertyAngle;
typedef Property<Area>			PropertyArea;
typedef Property<Charge>		PropertyCharge;
typedef Property<Current>		PropertyCurrent;
typedef Property<Density>		PropertyDensity;
typedef Property<Energy>		PropertyEnergy;
typedef Property<Force>			PropertyForce;
typedef Property<Power>			PropertyPower;
typedef Property<Pressure>		PropertyPressure;
typedef Property<Frequency>		PropertyFrequency;
typedef Property<Length>		PropertyLength;
typedef Property<Speed>			PropertySpeed;
typedef Property<Temperature>	PropertyTemperature;
typedef Property<Time>			PropertyTime;
typedef Property<Torque>		PropertyTorque;
typedef Property<Volume>		PropertyVolume;
typedef Property<Mass>			PropertyMass;

} // namespace v1

#endif

