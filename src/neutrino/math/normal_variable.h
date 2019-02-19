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

#ifndef NEUTRINO__MATH__NORMAL_VARIABLE_H__INCLUDED
#define NEUTRINO__MATH__NORMAL_VARIABLE_H__INCLUDED

// Standard:
#include <cstddef>


namespace neutrino {

/**
 * A pair of values, mean (expected value) and standard deviation.
 */
template<class pValue>
	class NormalVariable
	{
	  public:
		using Value = pValue;

	  public:
		// Ctor
		constexpr
		NormalVariable (Value mean, Value stddev);

		// Ctor
		constexpr
		NormalVariable (NormalVariable const&) = default;

		// Copy operator
		constexpr NormalVariable&
		operator= (NormalVariable const&) = default;

		constexpr Value
		mean() const noexcept
			{ return _mean; }

		constexpr Value
		stddev() const noexcept
			{ return _stddev; }

	  private:
		Value	_mean	{ };
		Value	_stddev	{ };
	};


template<class V>
	constexpr
	NormalVariable<V>::NormalVariable (Value mean, Value stddev):
		_mean (mean),
		_stddev (stddev)
	{ }

} // namespace neutrino

#endif

