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

#ifndef SI__OPERATORS_H__INCLUDED
#define SI__OPERATORS_H__INCLUDED

// Standard:
#include <cstddef>


namespace SI {

inline constexpr Time
operator/ (Length const& length, Speed const& speed)
{
	return 1_h * (length.nm() / speed.kt());
}


inline constexpr Speed
operator/ (Length const& length, Time const& time)
{
	return 1_kt * (length.nm() / time.h());
}


inline constexpr Length
operator* (Speed const& speed, Time const& time)
{
	return 1_nm * (speed.kt() * time.h());
}


inline constexpr Length
operator* (Time const& time, Speed const& speed)
{
	return speed * time;
}


inline constexpr Frequency
operator/ (double value, Time const& time)
{
	return 1_Hz * (value / time.s());
}


inline constexpr Time
operator/ (double value, Frequency const& frequency)
{
	return 1_s * (value / frequency.Hz());
}

} // namespace SI

#endif

