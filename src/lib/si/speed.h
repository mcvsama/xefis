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
	 * To create a Speed use these operators directly.
	 */
	explicit constexpr
	Speed (ValueType kt) noexcept;

  public:
	constexpr
	Speed() noexcept = default;

	constexpr
	Speed (Speed const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	kt() const noexcept;

	constexpr ValueType
	kph() const noexcept;

	constexpr ValueType
	fpm() const noexcept;

	constexpr ValueType
	mps() const noexcept;

	void
	set_si_units (ValueType) override;

	void
	parse (std::string const&) override;

	std::string
	stringify() const override;

	double
	floatize (std::string unit) const override;

  private:
	static std::vector<std::string> _supported_units;
};


static_assert (std::is_literal_type<Speed>::value, "Speed must be a literal type");


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


/*
 * Speed implementation
 */


inline constexpr
Speed::Speed (ValueType kt) noexcept:
	LinearValue (kt)
{ }


inline std::vector<std::string> const&
Speed::supported_units() const
{
	return _supported_units;
}


inline Speed::ValueType
Speed::si_units() const noexcept
{
	return mps();
}


inline constexpr Speed::ValueType
Speed::kt() const noexcept
{
	return internal();
}


inline constexpr Speed::ValueType
Speed::kph() const noexcept
{
	return internal() * 1.852;
}


inline constexpr Speed::ValueType
Speed::fpm() const noexcept
{
	return internal() * 101.268591426;
}


inline constexpr Speed::ValueType
Speed::mps() const noexcept
{
	return internal() / 1.9438612860586;
}


inline void
Speed::set_si_units (ValueType units)
{
	*this = 1_mps * units;
}


inline void
Speed::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "kt")
		*this = p.first * 1_kt;
	else if (p.second == "kph")
		*this = p.first * 1_kph;
	else if (p.second == "fpm")
		*this = p.first * 1_fpm;
}


inline std::string
Speed::stringify() const
{
	return boost::lexical_cast<std::string> (kt()) + " kt";
}


inline double
Speed::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "kt")
		return kt();
	else if (unit == "kph")
		return kph();
	else if (unit == "fpm")
		return fpm();
	else
		throw UnsupportedUnit ("can't convert Speed to " + unit);
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


template<>
	class is_floating_point<SI::Speed>: public is_floating_point<SI::Speed::ValueType>
	{ };

} // namespace std

#endif

