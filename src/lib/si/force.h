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

#ifndef SI__FORCE_H__INCLUDED
#define SI__FORCE_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Force: public LinearValue<double, Force>
{
	friend class LinearValue<double, Force>;
	friend constexpr Force operator"" _N (long double);
	friend constexpr Force operator"" _N (unsigned long long);

  protected:
	explicit constexpr
	Force (ValueType N) noexcept;

  public:
	constexpr
	Force() noexcept = default;

	constexpr
	Force (Force const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	N() const noexcept;

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


static_assert (std::is_literal_type<Force>::value, "Force must be a literal type");


/*
 * Global functions
 */


inline constexpr Force
operator"" _N (long double N)
{
	return Force (static_cast<Force::ValueType> (N));
}


inline constexpr Force
operator"" _N (unsigned long long N)
{
	return Force (static_cast<Force::ValueType> (N));
}


/*
 * Force implementation
 */


inline constexpr
Force::Force (ValueType N) noexcept:
	LinearValue (N)
{ }


inline std::vector<std::string> const&
Force::supported_units() const
{
	return _supported_units;
}


inline Force::ValueType
Force::si_units() const noexcept
{
	return N();
}


inline constexpr Force::ValueType
Force::N() const noexcept
{
	return internal();
}


inline void
Force::set_si_units (ValueType units)
{
	*this = 1_N * units;
}


inline void
Force::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "n")
		*this = p.first * 1_N;
}


inline std::string
Force::stringify() const
{
	return boost::lexical_cast<std::string> (N()) + " N";
}


inline double
Force::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "n")
		return N();
	else
		throw UnsupportedUnit ("can't convert Force to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Force.
 * Forwards Force::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Force>: public numeric_limits<SI::Force::ValueType>
	{ };


template<>
	class is_floating_point<SI::Force>: public is_floating_point<SI::Force::ValueType>
	{ };

} // namespace std

#endif

