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

#ifndef SI__UNITS__JOULE_H__INCLUDED
#define SI__UNITS__JOULE_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "../linear_value.h"


namespace SI {

class Joule: public LinearValue<double, Joule>
{
	friend class LinearValue<double, Joule>;
	friend constexpr Joule operator"" _J (long double);
	friend constexpr Joule operator"" _J (unsigned long long);

  public:
	explicit constexpr
	Joule (ValueType J) noexcept;

	constexpr
	Joule() noexcept = default;

	constexpr
	Joule (Joule const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	J() const noexcept;

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


static_assert (std::is_literal_type<Joule>::value, "Joule must be a literal type");


/*
 * Global functions
 */


inline constexpr Joule
operator"" _J (long double J)
{
	return Joule (static_cast<Joule::ValueType> (J));
}


inline constexpr Joule
operator"" _J (unsigned long long J)
{
	return Joule (static_cast<Joule::ValueType> (J));
}


/*
 * Joule implementation
 */


inline constexpr
Joule::Joule (ValueType J) noexcept:
	LinearValue (J)
{ }


inline std::vector<std::string> const&
Joule::supported_units() const
{
	return _supported_units;
}


inline Joule::ValueType
Joule::si_units() const noexcept
{
	return J();
}


inline constexpr Joule::ValueType
Joule::J() const noexcept
{
	return internal();
}


inline void
Joule::set_si_units (ValueType units)
{
	*this = 1_J * units;
}


inline void
Joule::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "j")
		*this = p.first * 1_J;
}


inline std::string
Joule::stringify() const
{
	return boost::lexical_cast<std::string> (J()) + " J";
}


inline double
Joule::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "j")
		return J();
	else
		throw UnsupportedUnit ("can't convert Joule to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Joule.
 * Forwards Joule::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Joule>: public numeric_limits<SI::Joule::ValueType>
	{ };


template<>
	class is_floating_point<SI::Joule>: public is_floating_point<SI::Joule::ValueType>
	{ };

} // namespace std

#endif

