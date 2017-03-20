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

#ifndef XEFIS__CORE__V2__PROPERTY_H__INCLUDED
#define XEFIS__CORE__V2__PROPERTY_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdint>

// Boost:
#include <boost/variant.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/utility/time.h>
#include <xefis/utility/time_helper.h>
#include <xefis/utility/blob.h>


namespace x2 {
using namespace xf; // XXX

/**
 * Exception object thrown when trying to read a nil property.
 */
class NilProperty: public Exception
{
  public:
	// Ctor
	explicit
	NilProperty (PropertyPath const&);
};


template<class Value>
	class PropertyIn;

template<class Value>
	class PropertyOut;


/**
 * Virtual interface for all Property objects and for some mixin classes.
 */
class PropertyVirtualInterface
{
  public:
	/**
	 * Return true if property is nil.
	 * If a fallback-value is set, it will never return true.
	 */
	virtual bool
	is_nil() const noexcept = 0;

	/**
	 * Set property to the nil value.
	 */
	virtual void
	set_nil() = 0;

	/**
	 * Valid means not nil. Equivalent to !is_nil().
	 */
	virtual bool
	valid() const noexcept = 0;

	/**
	 * Alias for valid().
	 */
	virtual
	operator bool() const noexcept = 0;

	/**
	 * Ensure that property's value is up to date in this processing loop.  If it's coupled to a Module, that process()
	 * has been called, or if it's coupled to another property, then the value has been copied.
	 */
	virtual void
	fetch (Cycle const&) = 0;
};


/**
 * Base class for all Property* types.
 */
class BasicProperty: virtual public PropertyVirtualInterface
{
  public:
	// Used to tell if node value has changed:
	typedef uint64_t Serial;

  protected:
	/**
	 * Create Property that doesn't have any data-source yet and is not coupled to any module.
	 */
	explicit
	BasicProperty (std::string const& path);

	/**
	 * Create Property that's coupled by a Module.
	 *
	 * \param	owner
	 *			Owner object for this property. May be nullptr.
	 */
	explicit
	BasicProperty (Module* owner, std::string const& path);

  public:
	// Dtor
	virtual
	~BasicProperty() = default;

	/**
	 * Return timestamp of the value (time when it was modified).
	 */
	Time
	modification_timestamp() const noexcept;

	/**
	 * Return age of the value (time since it was last modified).
	 */
	Time
	modification_age() const noexcept;

	/**
	 * Return timestamp of the last non-nil value.
	 */
	Time
	valid_timestamp() const noexcept;

	/**
	 * Return age of the non-nil value (time since it was last set to a non-nil value).
	 * Setting a fallback-value will essentially mean setting not-nil.
	 */
	Time
	valid_age() const noexcept;

	/**
	 * Return property path.
	 */
	PropertyPath const&
	path() const noexcept;

	/**
	 * Return the serial value of the property.
	 * Serial value changes when property is updated.
	 */
	Serial
	serial() const noexcept;

	/**
	 * Return property owner (Module). May be nullptr.
	 */
	Module*
	owner() const;

  protected:
	Module*			_owner					= nullptr;
	PropertyPath	_path;
	Timestamp		_modification_timestamp	= 0_s;
	Timestamp		_valid_timestamp		= 0_s;
	Serial			_serial					= 0;
};


/**
 * Mixin base class for all PropertyIn<*>
 */
class BasicPropertyIn: virtual public PropertyVirtualInterface
{ };


/**
 * Mixin base class for all PropertyOut<*>
 */
class BasicPropertyOut: virtual public PropertyVirtualInterface
{ };


/**
 * Note: perhaps the *_age() methods should not use the timestamp of the set() call, but some timestamp provided from
 * outside, eg. some souce data sampling timestamp. That would be more proper from digital signal processing
 * perspective, but I guess it's OK enough as it is now.
 */
template<class pValue>
	class Property: public BasicProperty
	{
	  public:
		typedef pValue Value;

	  protected:
		using BasicProperty::BasicProperty;

	  public:
		/**
		 * Set new value.
		 */
		void
		set (Value);

		/**
		 * Set new value or set to nil, of Optional is empty.
		 */
		void
		set (Optional<Value>);

		/**
		 * Copy value (or nil-state) from other proprety.
		 */
		void
		set (Property<Value> const&);

		/**
		 * Alias for set (Value).
		 */
		Property const&
		operator= (Value);

		/**
		 * Alias for set (Optional<Value>)
		 */
		Property const&
		operator= (Optional<Value>);

		/**
		 * Return contained value.
		 * Throw exception NilProperty if value is nil and no fallback-value is et.
		 */
		Value const&
		get() const;

		/**
		 * Alias for get().
		 */
		Value const&
		operator*() const;

		/**
		 * Return Optional that has value or is empty, if this property is nil.
		 * If fallback-value is set, the returned Optional will contain the fall-back value, and will never be empty.
		 */
		Optional<Value>
		get_optional() const;

		/**
		 * Return property's value or argument if property is nil.
		 * If property has a fallback-value set, then value_or will never return its argument, it will fall back to the
		 * fallback-value first.
		 */
		Value
		value_or (Value fallback) const;

		/**
		 * Pointer accessor if contained object is pointer-dereferencable.
		 * Throws the same exception as get() under the same conditions.
		 */
		Value const*
		operator->() const;

		/**
		 * Set fallback-value to use when this property isn't connected to any other property or its value is nil.
		 * Property with a fallback-value will essentially be seen as it's never nil.
		 *
		 * Affects value-retrieving methods and their aliases: get(), get_optional(), is_nil(), *_timestamp(), *_age(),
		 * valid(), serial().
		 *
		 * Pass empty Optional (or boost::none) to remove the fallback-value.
		 */
		void
		set_fallback (Optional<Value>);

		// BasicProperty API
		bool
		is_nil() const noexcept override;

		// BasicProperty API
		void
		set_nil() override;

		// BasicProperty API
		bool
		valid() const noexcept override;

		// BasicProperty API
		operator bool() const noexcept override;

	  private:
		Optional<Value>	_value;
		Optional<Value>	_fallback_value;
	};


template<class pValue>
	class PropertyIn:
		public BasicPropertyIn,
		public Property<pValue>
	{
	  public:
		typedef pValue Value;

		/**
		 * Create Property that's coupled to given owner, but doesn't have any data source yet.
		 */
		explicit
		PropertyIn (Module* owner, std::string const& path);

		/**
		 * Same as PropertyIn (Module*, std::string), but additionally set up the fallback value.
		 */
		explicit
		PropertyIn (Module* owner, std::string const& path, Value&& fallback_value);

		// Dtor
		~PropertyIn();

		using Property<Value>::operator=;

		/**
		 * Set no data source for this property.
		 */
		void
		operator<< (std::nullptr_t);

		/**
		 * Set PropertyOut as a data source for this property.
		 */
		void
		operator<< (PropertyOut<Value>& other);

		// BasicProperty API
		void
		fetch (Cycle const&) override;

	  private:
		PropertyOut<Value>* _data_source = nullptr;
	};


template<class pValue>
	class PropertyOut:
		public BasicPropertyOut,
		public Property<pValue>
	{
	  public:
		typedef pValue Value;

		/**
		 * Create Property that's not coupled to any module and don't have any data source yet.
		 */
		explicit
		PropertyOut (std::string const& path);

		/**
		 * Create Property that's coupled to a module and set the module as data source.
		 */
		explicit
		PropertyOut (Module* owner_and_data_source, std::string const& path);

		using Property<Value>::operator=;

		/**
		 * Return true if any other property depends on this property.
		 */
		bool
		connected() const noexcept;

		/**
		 * Set this property as data source for the other property.
		 */
		void
		operator>> (PropertyIn<Value>& other);

		/**
		 * Set this property as data source for the other property.
		 */
		void
		operator>> (PropertyOut<Value>& other);

		/**
		 * Set no data source for this property.
		 */
		void
		operator<< (std::nullptr_t);

		/**
		 * Set PropertyOut as a data source for this property.
		 */
		void
		operator<< (PropertyOut<Value>& other);

		// BasicProperty API
		void
		fetch (Cycle const&) override;

	  private:
		boost::variant<std::nullptr_t, Module*, PropertyOut<Value>*> _data_source = nullptr;
	};


/*
 * NilProperty
 */


inline
NilProperty::NilProperty (PropertyPath const& path):
	Exception ("tried to read a nil property " + path.string())
{ }


/*
 * BasicProperty
 */


inline
BasicProperty::BasicProperty (std::string const& path):
	_path (path)
{ }


inline
BasicProperty::BasicProperty (Module* owner, std::string const& path):
	_owner (owner),
	_path (path)
{ }


inline Timestamp
BasicProperty::modification_timestamp() const noexcept
{
	return _modification_timestamp;
}


inline Time
BasicProperty::modification_age() const noexcept
{
	return TimeHelper::now() - modification_timestamp();
}


inline Timestamp
BasicProperty::valid_timestamp() const noexcept
{
	return _valid_timestamp;
}


inline Time
BasicProperty::valid_age() const noexcept
{
	return TimeHelper::now() - valid_timestamp();
}


inline PropertyPath const&
BasicProperty::path() const noexcept
{
	return _path;
}


inline BasicProperty::Serial
BasicProperty::serial() const noexcept
{
	return _serial;
}


inline Module*
BasicProperty::owner() const
{
	return _owner;
}


/*
 * Property
 */


template<class V>
	inline void
	Property<V>::set (Value value)
	{
		if (!_value || *_value != value)
		{
			_modification_timestamp = TimeHelper::now();
			_valid_timestamp = _modification_timestamp;
			_value = value;
			++_serial;
		}
	}


template<class V>
	inline void
	Property<V>::set (Optional<Value> value)
	{
		if (value)
			set (*value);
		else
			set_nil();
	}


template<class V>
	inline void
	Property<V>::set (Property<Value> const& value)
	{
		if (value)
			set (*value);
		else
			set_nil();
	}


template<class V>
	inline Property<V> const&
	Property<V>::operator= (Value value)
	{
		set (value);
		return *this;
	}


template<class V>
	inline Property<V> const&
	Property<V>::operator= (Optional<Value> value)
	{
		set (value);
		return *this;
	}


template<class V>
	inline typename Property<V>::Value const&
	Property<V>::get() const
	{
		if (_value)
			return *_value;
		else if (_fallback_value)
			return *_fallback_value;
		else
			throw NilProperty (path());
	}


template<class V>
	inline typename Property<V>::Value const&
	Property<V>::operator*() const
	{
		return get();
	}


template<class V>
	inline Optional<typename Property<V>::Value>
	Property<V>::get_optional() const
	{
		if (_value)
			return _value;
		else
			return _fallback_value;
	}


template<class V>
	inline typename Property<V>::Value
	Property<V>::value_or (Value fallback) const
	{
		if (_value)
			return *_value;
		else if (_fallback_value)
			return *_fallback_value;
		else
			return fallback;
	}


template<class V>
	inline typename Property<V>::Value const*
	Property<V>::operator->() const
	{
		return &get();
	}


template<class V>
	inline void
	Property<V>::set_fallback (Optional<Value> fallback_value)
	{
		if (_fallback_value != fallback_value)
		{
			_modification_timestamp = TimeHelper::now();
			_valid_timestamp = _modification_timestamp;
			_fallback_value = fallback_value;
			++_serial;
		}
	}


template<class V>
	inline bool
	Property<V>::is_nil() const noexcept
	{
		return !_value && !_fallback_value;
	}


template<class V>
	inline void
	Property<V>::set_nil()
	{
		if (_value)
		{
			_modification_timestamp = TimeHelper::now();
			_value = boost::none;
			++_serial;
		}
	}


template<class V>
	inline bool
	Property<V>::valid() const noexcept
	{
		return !is_nil();
	}


template<class V>
	inline
	Property<V>::operator bool() const noexcept
	{
		return valid();
	}


/*
 * PropertyIn
 */


template<class V>
	inline
	PropertyIn<V>::PropertyIn (Module* owner, std::string const& path):
		Property<V> (owner, path)
	{
		Module::ProcessingLoopAPI (this->owner()).register_input_property (this);
	}


template<class V>
	inline
	PropertyIn<V>::PropertyIn (Module* owner, std::string const& path, Value&& fallback_value):
		PropertyIn (owner, path)
	{
		this->set_fallback (std::forward<Value> (fallback_value));
	}


template<class V>
	inline
	PropertyIn<V>::~PropertyIn()
	{
		Module::ProcessingLoopAPI (this->owner()).unregister_input_property (this);
	}


template<class V>
	inline void
	PropertyIn<V>::operator<< (PropertyOut<Value>& other)
	{
		_data_source = &other;
	}


template<class V>
	inline void
	PropertyIn<V>::operator<< (std::nullptr_t)
	{
		_data_source = nullptr;
	}


template<class V>
	inline void
	PropertyIn<V>::fetch (Cycle const& cycle)
	{
		if (_data_source)
		{
			_data_source->fetch (cycle);
			this->set (_data_source->get_optional());
		}
		else
			this->set_nil();
	}


/*
 * PropertyOut
 */


template<class V>
	inline
	PropertyOut<V>::PropertyOut (std::string const& path):
		Property<V> (path)
	{ }


template<class V>
	inline
	PropertyOut<V>::PropertyOut (Module* owner_and_data_source, std::string const& path):
		Property<V> (owner_and_data_source, path)
	{
		_data_source = owner_and_data_source;
	}


template<class V>
	inline bool
	PropertyOut<V>::connected() const noexcept
	{
		// TODO
		return true;
	}


template<class V>
	inline void
	PropertyOut<V>::operator>> (PropertyIn<Value>& other)
	{
		other << *this;
	}


template<class V>
	inline void
	PropertyOut<V>::operator>> (PropertyOut<Value>& other)
	{
		other << *this;
	}


template<class V>
	inline void
	PropertyOut<V>::operator<< (std::nullptr_t)
	{
		_data_source = nullptr;
	}


template<class V>
	inline void
	PropertyOut<V>::operator<< (PropertyOut<Value>& other)
	{
		_data_source = &other;
	}


template<class V>
	inline void
	PropertyOut<V>::fetch (Cycle const& cycle)
	{
		// TODO measure how often is this called for real-aircraft config,
		// perhaps add a flag that the result is cached in current processing-loop.
		struct Fetcher: public boost::static_visitor<>
		{
			Fetcher (PropertyOut<Value>* property_out, Cycle const& cycle):
				_this (property_out),
				_cycle (cycle)
			{ }

			void operator() (std::nullptr_t) const
			{
				_this->set_nil();
			}

			void operator() (Module* data_source) const
			{
				Module::ProcessingLoopAPI (data_source).fetch_and_process (_cycle);
			}

			void operator() (PropertyOut<Value>* data_source) const
			{
				data_source->fetch (_cycle);
				_this->set (data_source->get_optional());
			}

		  private:
			PropertyOut<Value>*	_this;
			Cycle const&		_cycle;
		};

		boost::apply_visitor (Fetcher (this, cycle), _data_source);
	}

} // namespace x2

#endif

