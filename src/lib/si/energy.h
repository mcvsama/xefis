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

#ifndef SI__ENERGY_H__INCLUDED
#define SI__ENERGY_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Energy: public LinearValue<double, Energy>
{
	friend class LinearValue<double, Energy>;
	friend constexpr Energy operator"" _J (long double);
	friend constexpr Energy operator"" _J (unsigned long long);

  public:
	explicit constexpr
	Energy (ValueType J) noexcept;

	constexpr
	Energy() noexcept = default;

	constexpr
	Energy (Energy const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	/**
	 * Return Joules.
	 */
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


static_assert (std::is_literal_type<Energy>::value, "Energy must be a literal type");


/*
 * Global functions
 */


inline constexpr Energy
operator"" _J (long double J)
{
	return Energy (static_cast<Energy::ValueType> (J));
}


inline constexpr Energy
operator"" _J (unsigned long long J)
{
	return Energy(static_cast<Energy::ValueType> (J));
}


/*
 * Energy implementation
 */


inline constexpr
Energy::Energy (ValueType J) noexcept:
	LinearValue (J)
{ }


inline std::vector<std::string> const&
Energy::supported_units() const
{
	return _supported_units;
}


inline Energy::ValueType
Energy::si_units() const noexcept
{
	return J();
}


inline constexpr Energy::ValueType
Energy::J() const noexcept
{
	return internal();
}


inline void
Energy::set_si_units (ValueType units)
{
	*this = 1_J * units;
}


inline void
Energy::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "j")
		*this = p.first * 1_J;
}


inline std::string
Energy::stringify() const
{
	return boost::lexical_cast<std::string> (J()) + " J";
}


inline double
Energy::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "j")
		return J();
	else
		throw UnsupportedUnit ("can't convert Energy to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Energy.
 * Forwards Energy::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Energy>: public numeric_limits<SI::Energy::ValueType>
	{ };


template<>
	class is_floating_point<SI::Energy>: public is_floating_point<SI::Energy::ValueType>
	{ };

} // namespace std

#endif

