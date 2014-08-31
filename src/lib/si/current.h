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

#ifndef SI__CURRENT_H__INCLUDED
#define SI__CURRENT_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Current: public LinearValue<double, Current>
{
	friend class LinearValue<double, Current>;
	friend constexpr Current operator"" _A (long double);
	friend constexpr Current operator"" _A (unsigned long long);
	friend constexpr Current operator"" _mA (long double);
	friend constexpr Current operator"" _mA (unsigned long long);

  protected:
	explicit constexpr
	Current (ValueType A) noexcept;

  public:
	constexpr
	Current() noexcept = default;

	constexpr
	Current (Current const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	A() const noexcept;

	constexpr ValueType
	mA() const noexcept;

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


static_assert (std::is_literal_type<Current>::value, "Current must be a literal type");


/*
 * Global functions
 */


inline constexpr Current
operator"" _A (long double A)
{
	return Current (static_cast<Current::ValueType> (A));
}


inline constexpr Current
operator"" _A (unsigned long long A)
{
	return Current (static_cast<Current::ValueType> (A));
}


inline constexpr Current
operator"" _mA (long double mA)
{
	return Current (static_cast<Current::ValueType> (mA) / 1000.0);
}


inline constexpr Current
operator"" _mA (unsigned long long mA)
{
	return Current (static_cast<Current::ValueType> (mA) / 1000.0);
}


/*
 * Current implementation
 */


inline constexpr
Current::Current (ValueType A) noexcept:
	LinearValue (A)
{ }


inline std::vector<std::string> const&
Current::supported_units() const
{
	return _supported_units;
}


inline Current::ValueType
Current::si_units() const noexcept
{
	return A();
}


inline constexpr Current::ValueType
Current::A() const noexcept
{
	return internal();
}


inline constexpr Current::ValueType
Current::mA() const noexcept
{
	return internal() * 1000.0;
}


inline void
Current::set_si_units (ValueType units)
{
	*this = 1_A * units;
}


inline void
Current::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "a")
		*this = p.first * 1_A;
	else if (p.second == "ma")
		*this = p.first * 1_mA;
}


inline std::string
Current::stringify() const
{
	return boost::lexical_cast<std::string> (A()) + " A";
}


inline double
Current::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "a")
		return A();
	else if (unit == "ma")
		return mA();
	else
		throw UnsupportedUnit ("can't convert Current to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Current.
 * Forwards Current::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Current>: public numeric_limits<SI::Current::ValueType>
	{ };


template<>
	class is_floating_point<SI::Current>: public is_floating_point<SI::Current::ValueType>
	{ };

} // namespace std

#endif

