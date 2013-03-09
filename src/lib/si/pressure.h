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

#ifndef SI__PRESSURE_H__INCLUDED
#define SI__PRESSURE_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "value.h"


namespace SI {

class Pressure: public Value<float, Pressure>
{
	friend class Value<float, Pressure>;
	friend constexpr Pressure operator"" _psi (long double);
	friend constexpr Pressure operator"" _psi (unsigned long long);
	friend constexpr Pressure operator"" _hpa (long double);
	friend constexpr Pressure operator"" _hpa (unsigned long long);
	friend constexpr Pressure operator"" _inhg (long double);
	friend constexpr Pressure operator"" _inhg (unsigned long long);

  protected:
	/**
	 * Used by the _inhg, _hpa, _psi and similar suffix operators.
	 * To create an Pressure use these operators directly.
	 */
	constexpr
	Pressure (ValueType psi);

  public:
	constexpr
	Pressure() = default;

	constexpr
	Pressure (Pressure const&) = default;

	constexpr ValueType
	psi() const noexcept;

	constexpr ValueType
	hpa() const noexcept;

	constexpr ValueType
	inhg() const noexcept;
};


inline constexpr
Pressure::Pressure (ValueType psi):
	Value (psi)
{ }


inline constexpr Pressure::ValueType
Pressure::psi() const noexcept
{
	return value();
}


inline constexpr Pressure::ValueType
Pressure::hpa() const noexcept
{
	return value() * 68.9554630643f;
}


inline constexpr Pressure::ValueType
Pressure::inhg() const noexcept
{
	return value() * 2.036254f;
}


/*
 * Global functions
 */


inline constexpr Pressure
operator"" _psi (long double psi)
{
	return Pressure (static_cast<Pressure::ValueType> (psi));
}


inline constexpr Pressure
operator"" _psi (unsigned long long psi)
{
	return Pressure (static_cast<Pressure::ValueType> (psi));
}


inline constexpr Pressure
operator"" _hpa (long double hpa)
{
	return Pressure (static_cast<Pressure::ValueType> (hpa) * 0.0145021141);
}


inline constexpr Pressure
operator"" _hpa (unsigned long long hpa)
{
	return Pressure (static_cast<Pressure::ValueType> (hpa) * 0.0145021141);
}


inline constexpr Pressure
operator"" _inhg (long double inhg)
{
	return Pressure (static_cast<Pressure::ValueType> (inhg) * 0.491098f);
}


inline constexpr Pressure
operator"" _inhg (unsigned long long inhg)
{
	return Pressure (static_cast<Pressure::ValueType> (inhg) * 0.491098f);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Pressure.
 * Forwards Pressure::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Pressure>: public numeric_limits<SI::Pressure::ValueType>
	{ };

} // namespace std

#endif

