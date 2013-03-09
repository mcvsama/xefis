/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef SI__ANGLE_H__INCLUDED
#define SI__ANGLE_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <limits>

// Local:
#include "value.h"


namespace SI {

class Angle: public Value<double, Angle>
{
	friend class Value<double, Angle>;
	friend constexpr Angle operator"" _rad (long double);
	friend constexpr Angle operator"" _rad (unsigned long long);
	friend constexpr Angle operator"" _deg (long double);
	friend constexpr Angle operator"" _deg (unsigned long long);

  protected:
	/**
	 * Used by the _rad and _deg suffix operators.
	 * To create an Angle use these operators directly.
	 */
	constexpr
	Angle (ValueType radians);

  public:
	constexpr
	Angle() = default;

	constexpr
	Angle (Angle const&) = default;

	constexpr ValueType
	rad() const noexcept;

	constexpr ValueType
	deg() const noexcept;
};


inline constexpr
Angle::Angle (ValueType radians):
	Value (radians)
{ }


inline constexpr Angle::ValueType
Angle::rad() const noexcept
{
	return value();
}


inline constexpr Angle::ValueType
Angle::deg() const noexcept
{
	return value() * 180.0 / M_PI;
}


/*
 * Global functions
 */


inline constexpr Angle
operator"" _rad (long double radians)
{
	return Angle (static_cast<Angle::ValueType> (radians));
}


inline constexpr Angle
operator"" _rad (unsigned long long radians)
{
	return Angle (static_cast<Angle::ValueType> (radians));
}


inline constexpr Angle
operator"" _deg (long double degrees)
{
	return Angle (static_cast<Angle::ValueType> (degrees * M_PI / 180.0));
}


inline constexpr Angle
operator"" _deg (unsigned long long degrees)
{
	return Angle (static_cast<Angle::ValueType> (degrees) * M_PI / 180.0);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Angle.
 * Forwards Angle::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Angle>: public numeric_limits<SI::Angle::ValueType>
	{ };

} // namespace std

#endif

