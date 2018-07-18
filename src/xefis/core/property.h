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
#include <cstdint>
#include <optional>
#include <variant>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/module_io.h>
#include <xefis/core/property_path.h>
#include <xefis/utility/blob.h>
#include <xefis/utility/time.h>
#include <xefis/utility/time_helper.h>


namespace xf {

class PropertyStringConverter;


/**
 * Helper type that indicates Nil values for properties.
 */
class Nil
{ };


/**
 * Global nil object that when compared to a nil property, gives true.
 */
static constexpr Nil nil;


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


/**
 * Virtual interface for all Property objects and for some mixin classes.
 */
class PropertyVirtualInterface
{
  public:
	// Used to tell if node value has changed:
	typedef uint64_t Serial;

  public:
	// Dtor
	virtual
	~PropertyVirtualInterface() = default;

	/**
	 * Return true if property is nil.
	 * If a fallback-value is set, it will never return true.
	 */
	virtual bool
	is_nil() const noexcept = 0;

	/**
	 * Alias for is_nil().
	 */
	bool
	operator== (Nil) const
	{
		return is_nil();
	}

	/**
	 * Valid means not nil. Equivalent to !is_nil().
	 */
	virtual bool
	valid() const noexcept = 0;

	/**
	 * Alias for valid().
	 */
	operator bool() const noexcept
	{
		return valid();
	}

	/**
	 * Return timestamp of the value (time when it was modified).
	 */
	virtual si::Time
	modification_timestamp() const noexcept = 0;

	/**
	 * Return age of the value (time since it was last modified).
	 */
	virtual si::Time
	modification_age() const noexcept = 0;

	/**
	 * Return timestamp of the last non-nil value.
	 */
	virtual si::Time
	valid_timestamp() const noexcept = 0;

	/**
	 * Return age of the non-nil value (time since it was last set to a non-nil value).
	 * Setting a fallback-value will essentially mean setting not-nil.
	 */
	virtual si::Time
	valid_age() const noexcept = 0;

	/**
	 * Return property path.
	 */
	virtual PropertyPath const&
	path() const noexcept = 0;

	/**
	 * Return the serial value of the property.
	 * Serial value changes when property is updated.
	 */
	virtual Serial
	serial() const noexcept = 0;

	/**
	 * Ensure that property's value is up to date in this processing loop.
	 */
	virtual void
	fetch (Cycle const&) = 0;

	/**
	 * Serializes property value, including nil-flag.
	 * The blob has variable-length.
	 */
	virtual void
	property_to_blob (Blob&) const = 0;

	/**
	 * Convenience overload that returns the Blob object.
	 */
	Blob
	property_to_blob() const
	{
		Blob result;
		property_to_blob (result);
		return result;
	}

	/**
	 * Deserializes property value.
	 * The blob has variable-length.
	 * \throw	InvalidBlobSize
	 *			If blob has size not corresponding to this property type.
	 */
	virtual void
	blob_to_property (Blob const&) = 0;

	/**
	 * Increase use-count for this property.
	 */
	virtual void
	inc_use_count() noexcept = 0;

	/**
	 * Decrease use-count for this property.
	 */
	virtual void
	dec_use_count() noexcept = 0;

	/**
	 * Use-count for this property.
	 */
	virtual std::size_t
	use_count() const noexcept = 0;

	/**
	 * Get new string converter associated with this property.
	 */
	virtual PropertyStringConverter
	get_string_converter() = 0;

  protected:
	/**
	 * Set property to the nil value.
	 */
	virtual void
	protected_set_nil() = 0;
};


/**
 * Base class for all Property* types.
 */
class BasicProperty: virtual public PropertyVirtualInterface
{
  protected:
	/**
	 * Create Property that doesn't have any data-source yet and is not coupled to any module.
	 */
	explicit
	BasicProperty (std::string const& path);

	/**
	 * Create Property that's coupled by a ModuleIO.
	 *
	 * \param	owner
	 *			Owner object for this property. May be nullptr.
	 */
	explicit
	BasicProperty (ModuleIO* owner, std::string const& path);

  public:
	// Dtor
	virtual
	~BasicProperty() = default;

	/**
	 * Return property owner (an ModuleIO object). May be nullptr.
	 */
	ModuleIO*
	io() const noexcept;

	// PropertyVirtualInterface API
	si::Time
	modification_timestamp() const noexcept override;

	// PropertyVirtualInterface API
	si::Time
	modification_age() const noexcept override;

	// PropertyVirtualInterface API
	si::Time
	valid_timestamp() const noexcept override;

	// PropertyVirtualInterface API
	si::Time
	valid_age() const noexcept override;

	// PropertyVirtualInterface API
	PropertyPath const&
	path() const noexcept override;

	// PropertyVirtualInterface API
	Serial
	serial() const noexcept override;

	// PropertyVirtualInterface API
	void
	inc_use_count() noexcept override;

	void
	dec_use_count() noexcept override;

	std::size_t
	use_count() const noexcept override;

  protected:
	ModuleIO*		_owner					= nullptr;
	PropertyPath	_path;
	si::Time		_modification_timestamp	= 0_s;
	si::Time		_valid_timestamp		= 0_s;
	Serial			_serial					= 0;
	std::size_t		_use_count				= 0;
};


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
		 * Return contained value.
		 * Throw exception NilProperty if value is nil and no fallback-value is set.
		 */
		Value const&
		get() const;

		/**
		 * Alias for get().
		 */
		Value const&
		operator*() const;

		/**
		 * Return std::optional that has value or is empty, if this property is nil.
		 * If fallback-value is set, the returned std::optional will contain the fall-back value, and will never be empty.
		 */
		std::optional<Value>
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
		 * Pass empty std::optional (or std::nullopt) to remove the fallback-value.
		 */
		void
		set_fallback (std::optional<Value>);

		// BasicProperty API
		bool
		is_nil() const noexcept override;

		// BasicProperty API
		bool
		valid() const noexcept override;

		// BasicProperty API
		void
		property_to_blob (Blob&) const;

		// BasicProperty API
		void
		blob_to_property (Blob const&);

		// PropertyVirtualInterface API
		PropertyStringConverter
		get_string_converter() override;

	  protected:
		/**
		 * Set new value.
		 */
		void
		protected_set_value (Value);

		/**
		 * Set new value or set to nil, of std::optional is empty.
		 */
		void
		protected_set (std::optional<Value>);

		/**
		 * Copy value (or nil-state) from other proprety.
		 */
		void
		protected_set (Property<Value> const&);

		// BasicProperty API
		void
		protected_set_nil() override;

	  private:
		std::optional<Value>	_value;
		std::optional<Value>	_fallback_value;
	};

} // namespace xf


#include "property.tcc"
#include "property_in.h"
#include "property_out.h"

#endif

