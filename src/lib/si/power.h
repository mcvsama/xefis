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

#ifndef SI__POWER_H__INCLUDED
#define SI__POWER_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Power: public LinearValue<double, Power>
{
	friend class LinearValue<double, Power>;
	friend constexpr Power operator"" _W (long double);
	friend constexpr Power operator"" _W (unsigned long long);

  public:
	explicit constexpr
	Power (ValueType W) noexcept;

	constexpr
	Power() noexcept = default;

	constexpr
	Power (Power const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	/**
	 * Return Watts.
	 */
	constexpr ValueType
	W() const noexcept;

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


static_assert (std::is_literal_type<Power>::value, "Power must be a literal type");


/*
 * Global functions
 */


inline constexpr Power
operator"" _W (long double W)
{
	return Power (static_cast<Power::ValueType> (W));
}


inline constexpr Power
operator"" _W (unsigned long long W)
{
	return Power(static_cast<Power::ValueType> (W));
}


/*
 * Power implementation
 */


inline constexpr
Power::Power (ValueType W) noexcept:
	LinearValue (W)
{ }


inline std::vector<std::string> const&
Power::supported_units() const
{
	return _supported_units;
}


inline Power::ValueType
Power::si_units() const noexcept
{
	return W();
}


inline constexpr Power::ValueType
Power::W() const noexcept
{
	return internal();
}


inline void
Power::set_si_units (ValueType units)
{
	*this = 1_W * units;
}


inline void
Power::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "w")
		*this = p.first * 1_W;
}


inline std::string
Power::stringify() const
{
	return boost::lexical_cast<std::string> (W()) + " W";
}


inline double
Power::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "w")
		return W();
	else
		throw UnsupportedUnit ("can't convert Power to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Power.
 * Forwards Power::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Power>: public numeric_limits<SI::Power::ValueType>
	{ };


template<>
	class is_floating_point<SI::Power>: public is_floating_point<SI::Power::ValueType>
	{ };

} // namespace std

#endif

