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

#ifndef XEFIS__UTILITY__TEMPORAL_H__INCLUDED
#define XEFIS__UTILITY__TEMPORAL_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

template<class tValueType>
	class Temporal
	{
	  public:
		typedef tValueType ValueType;

	  public:
		// Ctor
		Temporal (ValueType const& value) noexcept (noexcept (ValueType (value)));

		// Ctor
		Temporal (ValueType const& value, Time const& update_time) noexcept (noexcept (ValueType (value)));

		// Copy ctor
		Temporal (Temporal const&) = default;

		// Move ctor
		Temporal (Temporal&&) = default;

		// Copy assignment
		Temporal<ValueType>&
		operator= (Temporal<ValueType> const&) = default;

		/**
		 * Assign new value with current timestamp
		 * (taken from Time::now()).
		 */
		Temporal<ValueType>&
		operator= (ValueType const& value) noexcept (noexcept (value = value));

		/**
		 * Assign new value with given timestamp.
		 */
		void
		set (ValueType const& value, Time const& update_time) noexcept (noexcept (value = value));

		/**
		 * Return contained value.
		 */
		ValueType const&
		value() const noexcept;

		/**
		 * Return update timestamp.
		 */
		Time const&
		update_time() const noexcept;

	  private:
		ValueType		_value;
		Time			_update_time;
	};


template<class T>
	inline
	Temporal<T>::Temporal (ValueType const& value) noexcept (noexcept (ValueType (value))):
		_value (value),
		_update_time (Time::now())
	{ }


template<class T>
	inline
	Temporal<T>::Temporal (ValueType const& value, Time const& update_time) noexcept (noexcept (ValueType (value))):
		_value (value),
		_update_time (update_time)
	{ }


template<class T>
	inline Temporal<T>&
	Temporal<T>::operator= (ValueType const& value) noexcept (noexcept (value = value))
	{
		_value = value;
		_update_time = Time::now();
	}


template<class T>
	inline void
	Temporal<T>::set (ValueType const& value, Time const& update_time) noexcept (noexcept (value = value))
	{
		_value = value;
		_update_time = update_time;
	}


template<class T>
	inline typename Temporal<T>::ValueType const&
	Temporal<T>::value() const noexcept
	{
		return _value;
	}


template<class T>
	inline Time const&
	Temporal<T>::update_time() const noexcept
	{
		return _update_time;
	}

} // namespace Xefis

#endif

