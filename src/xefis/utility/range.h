/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__UTILITY__RANGE_H__INCLUDED
#define XEFIS__UTILITY__RANGE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

template<class tValueType>
	class Range
	{
	  public:
		typedef tValueType ValueType;

	  public:
		constexpr
		Range() noexcept = default;

		constexpr
		Range (ValueType min, ValueType max) noexcept;

		constexpr
		Range (Range const& other) noexcept;

		template<class OtherType>
			constexpr
			operator Range<OtherType>() const noexcept
			{
				return Range<OtherType> (_min, _max);
			}

		constexpr ValueType
		min() const noexcept;

		constexpr ValueType
		max() const noexcept;

		void
		set_min (ValueType min) noexcept;

		void
		set_max (ValueType max) noexcept;

		/**
		 * Return maximum() - minimum().
		 */
		constexpr ValueType
		extent() const noexcept;

		/**
		 * Swap minimum and maximum values.
		 */
		void
		flip();

		/**
		 * Return a copy with swapped minimum and maximum values.
		 */
		constexpr Range
		flipped() const;

		/**
		 * Return true if given value fits inside range,
		 * inclusively.
		 */
		constexpr bool
		includes (ValueType) const;

	  private:
		ValueType	_min	= ValueType();
		ValueType	_max	= ValueType();
	};


template<class T>
	constexpr
	Range<T>::Range (ValueType min, ValueType max) noexcept:
		_min (min),
		_max (max)
	{ }


template<class T>
	constexpr
	Range<T>::Range (Range<T> const& other) noexcept:
		_min (other._min),
		_max (other._max)
	{ }


template<class T>
	constexpr
	typename Range<T>::ValueType
	Range<T>::min() const noexcept
	{
		return _min;
	}


template<class T>
	constexpr
	typename Range<T>::ValueType
	Range<T>::max() const noexcept
	{
		return _max;
	}


template<class T>
	void
	Range<T>::set_min (ValueType min) noexcept
	{
		_min = min;
	}


template<class T>
	void
	Range<T>::set_max (ValueType max) noexcept
	{
		_max = max;
	}


template<class T>
	constexpr
	typename Range<T>::ValueType
	Range<T>::extent() const noexcept
	{
		return _max - _min;
	}


template<class T>
	void
	Range<T>::flip()
	{
		std::swap (_min, _max);
	}


template<class T>
	constexpr Range<T>
	Range<T>::flipped() const
	{
		return Range { _max, _min };
	}


template<class T>
	constexpr bool
	Range<T>::includes (ValueType value) const
	{
		return _min <= _max
			? (_min <= value && value <= _max)
			: (_max <= value && value <= _min);
	}

} // namespace Xefis

#endif

