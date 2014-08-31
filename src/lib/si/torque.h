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

#ifndef SI__TORQUE_H__INCLUDED
#define SI__TORQUE_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <limits>

// Boost:
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

// Local:
#include "linear_value.h"


namespace SI {

class Torque: public LinearValue<double, Torque>
{
	friend class LinearValue<double, Torque>;
	friend constexpr Torque operator"" _Nm (long double);
	friend constexpr Torque operator"" _Nm (unsigned long long);

  protected:
	/**
	 * Used by the suffix operators.
	 * To create a Torque use those operators directly.
	 */
	explicit constexpr
	Torque (ValueType newton_meters) noexcept;

  public:
	constexpr
	Torque() noexcept = default;

	constexpr
	Torque (Torque const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	Nm() const noexcept;

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


static_assert (std::is_literal_type<Torque>::value, "Torque must be a literal type");


/*
 * Global functions
 */


inline constexpr Torque
operator"" _Nm (long double nm)
{
	return Torque (static_cast<Torque::ValueType> (nm));
}


inline constexpr Torque
operator"" _Nm (unsigned long long nm)
{
	return Torque (static_cast<Torque::ValueType> (nm));
}


/*
 * Torque implementation
 */


inline constexpr
Torque::Torque (ValueType radians) noexcept:
	LinearValue (radians)
{ }


inline std::vector<std::string> const&
Torque::supported_units() const
{
	return _supported_units;
}


inline Torque::ValueType
Torque::si_units() const noexcept
{
	return Nm();
}


inline constexpr Torque::ValueType
Torque::Nm() const noexcept
{
	return internal();
}


inline void
Torque::set_si_units (ValueType units)
{
	*this = 1_Nm * units;
}


inline void
Torque::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "nm")
		*this = p.first * 1_Nm;
}


inline std::string
Torque::stringify() const
{
	return boost::lexical_cast<std::string> (Nm()) + " Nm";
}


inline double
Torque::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "nm")
		return Nm();
	else
		throw UnsupportedUnit ("can't convert Torque to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Torque.
 * Forwards Torque::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Torque>: public numeric_limits<SI::Torque::ValueType>
	{ };


template<>
	class is_floating_point<SI::Torque>: public is_floating_point<SI::Torque::ValueType>
	{ };

} // namespace std

#endif

