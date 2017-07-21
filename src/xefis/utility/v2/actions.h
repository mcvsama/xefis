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

#ifndef XEFIS__UTILITY__V2__ACTIONS_H__INCLUDED
#define XEFIS__UTILITY__V2__ACTIONS_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace v2 {

/**
 * Checks if an action should be executed based on value of some properties and saved state.
 */
class PropAction
{
  public:
	/**
	 * Tell whether condition was met to execute an action.
	 */
	virtual bool
	operator()() = 0;
};


/**
 * Checks whether a property changed its value since last check.
 */
template<class pValue>
	class PropChanged: public PropAction
	{
	  public:
		using Value			= pValue;
		using OptionalValue	= Optional<Value>;
		using Property		= v2::Property<Value>;

	  public:
		// Ctor
		explicit
		PropChanged (Property& property):
			_property (property),
			_last_value (property.get_optional())
		{ }

		// PropAction API
		bool
		operator()() override
		{
			auto current_value = _property.get_optional();

			if (_last_value != current_value)
			{
				_last_value = current_value;
				return true;
			}

			return false;
		}

		/**
		 * Return reference to observed property.
		 */
		Property const&
		property() const noexcept
		{
			return _property;
		}

		/**
		 * Return reference to observed property.
		 */
		Property&
		property() noexcept
		{
			return _property;
		}

	  private:
		Property&		_property;
		OptionalValue	_last_value;
	};


/**
 * Checks whether a property has changed and has now given value.
 */
template<class pValue>
	class PropChangedTo: public PropChanged<pValue>
	{
	  public:
		using Value			= pValue;
		using OptionalValue	= typename PropChanged<Value>::OptionalValue;
		using Property		= typename PropChanged<Value>::Property;

	  public:
		// Ctor
		constexpr
		PropChangedTo (Property& property, Value value):
			PropChanged<Value> (property),
			_expected_value (value)
		{ }

		// PropAction API
		bool
		operator()() override
		{
			return PropChanged<Value>::operator()()
				&& this->property().valid()
				&& *this->property() == expected_value();
		}

		/**
		 * Return the value this observer expects.
		 */
		constexpr Value
		expected_value() const noexcept
		{
			return _expected_value;
		}

	  private:
		Value _expected_value;
	};


/**
 * Checks whether a property changed to nil.
 */
template<class pValue>
	class ChangedToNil: public PropChanged<pValue>
	{
	  public:
		using Value			= pValue;
		using OptionalValue	= typename PropChanged<Value>::OptionalValue;
		using Property		= typename PropChanged<Value>::Property;

	  public:
		using PropChanged<Value>::PropChanged;

		// PropAction API
		bool
		operator()() override
		{
			return PropChanged<Value>::operator()
				&& this->property().is_nil();
		}
	};

} // namespace v2

#endif

