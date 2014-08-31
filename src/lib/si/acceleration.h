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

#ifndef SI__ACCELERATION_H__INCLUDED
#define SI__ACCELERATION_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Acceleration: public LinearValue<double, Acceleration>
{
	friend class LinearValue<double, Acceleration>;
	friend constexpr Acceleration operator"" _mps2 (long double);
	friend constexpr Acceleration operator"" _mps2 (unsigned long long);
	friend constexpr Acceleration operator"" _g (long double);
	friend constexpr Acceleration operator"" _g (unsigned long long);

  protected:
	explicit constexpr
	Acceleration (ValueType mps2) noexcept;

  public:
	constexpr
	Acceleration() noexcept = default;

	constexpr
	Acceleration (Acceleration const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	mps2() const noexcept;

	constexpr ValueType
	g() const noexcept;

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


static_assert (std::is_literal_type<Acceleration>::value, "Acceleration must be a literal type");


/*
 * Global functions
 */


inline constexpr Acceleration
operator"" _mps2 (long double mps2)
{
	return Acceleration (static_cast<Acceleration::ValueType> (mps2));
}


inline constexpr Acceleration
operator"" _mps2 (unsigned long long mps2)
{
	return Acceleration (static_cast<Acceleration::ValueType> (mps2));
}


inline constexpr Acceleration
operator"" _g (long double g)
{
	return Acceleration (static_cast<Acceleration::ValueType> (g) * 9.80665);
}


inline constexpr Acceleration
operator"" _g (unsigned long long g)
{
	return Acceleration (static_cast<Acceleration::ValueType> (g) * 9.80665);
}


/*
 * Acceleration implementation
 */


inline constexpr
Acceleration::Acceleration (ValueType mps2) noexcept:
	LinearValue (mps2)
{ }


inline std::vector<std::string> const&
Acceleration::supported_units() const
{
	return _supported_units;
}


inline Acceleration::ValueType
Acceleration::si_units() const noexcept
{
	return mps2();
}


inline constexpr Acceleration::ValueType
Acceleration::mps2() const noexcept
{
	return internal();
}


inline constexpr Acceleration::ValueType
Acceleration::g() const noexcept
{
	return mps2() / 9.80665;
}


inline void
Acceleration::set_si_units (ValueType units)
{
	*this = 1_mps2 * units;
}


inline void
Acceleration::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "g")
		*this = p.first * 1_g;
	else if (p.second == "m/s2" || p.second == "m/s²" || p.second == "mps2")
		*this = p.first * 1_mps2;
}


inline std::string
Acceleration::stringify() const
{
	return boost::lexical_cast<std::string> (mps2()) + " m/s²";
}


inline double
Acceleration::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "g")
		return g();
	else if (unit == "m/s2" || unit == "m/s²" || unit == "mps2")
		return mps2();
	else
		throw UnsupportedUnit ("can't convert Acceleration to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Acceleration.
 * Forwards Acceleration::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Acceleration>: public numeric_limits<SI::Acceleration::ValueType>
	{ };


template<>
	class is_floating_point<SI::Acceleration>: public is_floating_point<SI::Acceleration::ValueType>
	{ };

} // namespace std

#endif

