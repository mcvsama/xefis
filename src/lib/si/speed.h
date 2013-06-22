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

#ifndef SI__SPEED_H__INCLUDED
#define SI__SPEED_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Speed: public LinearValue<double, Speed>
{
	friend class LinearValue<double, Speed>;
	friend constexpr Speed operator"" _kt (long double);
	friend constexpr Speed operator"" _kt (unsigned long long);
	friend constexpr Speed operator"" _kph (long double);
	friend constexpr Speed operator"" _kph (unsigned long long);
	friend constexpr Speed operator"" _fpm (long double);
	friend constexpr Speed operator"" _fpm (unsigned long long);
	friend constexpr Speed operator"" _mps (long double);
	friend constexpr Speed operator"" _mps (unsigned long long);

  protected:
	/**
	 * Used by the _rad and _deg suffix operators.
	 * To create an Speed use these operators directly.
	 */
	explicit constexpr
	Speed (ValueType kt);

  public:
	constexpr
	Speed() = default;

	constexpr
	Speed (Speed const&) = default;

	std::vector<std::string> const&
	supported_units() const override;

	constexpr ValueType
	kt() const noexcept;

	constexpr ValueType
	kph() const noexcept;

	constexpr ValueType
	fpm() const noexcept;

	constexpr ValueType
	mps() const noexcept;

	Speed&
	parse (std::string const&);

	std::string
	stringify() const;

  private:
	static std::vector<std::string> _supported_units;
};


inline constexpr
Speed::Speed (ValueType kt):
	LinearValue (kt)
{ }


inline std::vector<std::string> const&
Speed::supported_units() const
{
	return _supported_units;
}


inline constexpr Speed::ValueType
Speed::kt() const noexcept
{
	return value();
}


inline constexpr Speed::ValueType
Speed::kph() const noexcept
{
	return value() * 1.852;
}


inline constexpr Speed::ValueType
Speed::fpm() const noexcept
{
	return value() * 101.268591426;
}


inline constexpr Speed::ValueType
Speed::mps() const noexcept
{
	return value() / 1.9438612860586;
}


inline Speed&
Speed::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "kt")
		*this = p.first * 1_kt;
	else if (p.second == "kph")
		*this = p.first * 1_kph;
	else if (p.second == "fpm")
		*this = p.first * 1_fpm;

	return *this;
}


inline std::string
Speed::stringify() const
{
	return boost::lexical_cast<std::string> (kt()) + " kt";
}


/*
 * Global functions
 */


inline constexpr Speed
operator"" _kt (long double knots)
{
	return Speed (static_cast<Speed::ValueType> (knots));
}


inline constexpr Speed
operator"" _kt (unsigned long long knots)
{
	return Speed (static_cast<Speed::ValueType> (knots));
}


inline constexpr Speed
operator"" _kph (long double kph)
{
	return Speed (static_cast<Speed::ValueType> (kph / 1.852));
}


inline constexpr Speed
operator"" _kph (unsigned long long kph)
{
	return Speed (static_cast<Speed::ValueType> (kph / 1.852));
}


inline constexpr Speed
operator"" _fpm (long double fpm)
{
	return Speed (static_cast<Speed::ValueType> (fpm / 101.268591426));
}


inline constexpr Speed
operator"" _fpm (unsigned long long fpm)
{
	return Speed (static_cast<Speed::ValueType> (fpm / 101.268591426));
}


inline constexpr Speed
operator"" _mps (long double mps)
{
	return Speed (static_cast<Speed::ValueType> (mps * 1.9438612860586));
}


inline constexpr Speed
operator"" _mps (unsigned long long mps)
{
	return Speed (static_cast<Speed::ValueType> (mps * 1.9438612860586));
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Speed.
 * Forwards Speed::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Speed>: public numeric_limits<SI::Speed::ValueType>
	{ };

} // namespace std

#endif

