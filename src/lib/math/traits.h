/* vim:ts=4
 *
 * Copyleft 2012…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef MATH__TRAITS_H__INCLUDED
#define MATH__TRAITS_H__INCLUDED

// Standard:
#include <cstddef>
#include <type_traits>


namespace math {

template<class pValue>
	struct Traits
	{
		typedef pValue Value;

		static Value
		zero();

		static Value
		one();

		static Value
		inversed (Value const&);
	};


template<class Value>
	inline Value
	Traits<Value>::zero()
	{
		return 0;
	}


template<class Value>
	inline Value
	Traits<Value>::one()
	{
		return 1;
	}


template<class Value>
	inline Value
	Traits<Value>::inversed (Value const& v)
	{
		return one() / v;
	}


/*
 * General functions
 */


template<class Value>
	Value
	inversed (Value const& value)
	{
		return Traits<Value>::inversed (value);
	}

} // namespace math

#endif

