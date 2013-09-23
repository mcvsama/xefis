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

#ifndef SI__TEMPERATURE_H__INCLUDED
#define SI__TEMPERATURE_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Temperature: public LinearValue<double, Temperature>
{
	friend class LinearValue<double, Temperature>;
	friend constexpr Temperature operator"" _K (long double);
	friend constexpr Temperature operator"" _K (unsigned long long);
	friend constexpr Temperature operator"" _degC (long double);
	friend constexpr Temperature operator"" _degC (unsigned long long);
	friend constexpr Temperature operator"" _degF (long double);
	friend constexpr Temperature operator"" _degF (unsigned long long);

  protected:
	/**
	 * Used by the suffix operators.
	 */
	explicit constexpr
	Temperature (ValueType kelvins);

  public:
	constexpr
	Temperature() = default;

	constexpr
	Temperature (Temperature const&) = default;

	std::vector<std::string> const&
	supported_units() const override;

	constexpr ValueType
	K() const noexcept;

	constexpr ValueType
	degC() const noexcept;

	constexpr ValueType
	degF() const noexcept;

	void
	parse (std::string const&) override;

	std::string
	stringify() const override;

	double
	floatize (std::string unit) const override;

  private:
	static std::vector<std::string> _supported_units;
};


inline constexpr
Temperature::Temperature (ValueType kelvins):
	LinearValue (kelvins)
{ }


inline std::vector<std::string> const&
Temperature::supported_units() const
{
	return _supported_units;
}


inline constexpr Temperature::ValueType
Temperature::K() const noexcept
{
	return internal();
}


inline constexpr Temperature::ValueType
Temperature::degC() const noexcept
{
	return internal() - 273.15;
}


inline constexpr Temperature::ValueType
Temperature::degF() const noexcept
{
	return degC() * 1.8 + 32.0;
}


inline void
Temperature::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "k")
		*this = p.first * 1_K;
	else if (p.second == "c" || p.second == "degc" || p.second == "°c")
		*this = p.first * 1_degC;
	else if (p.second == "f" || p.second == "degf" || p.second == "°f")
		*this = p.first * 1_degF;
}


inline std::string
Temperature::stringify() const
{
	return boost::lexical_cast<std::string> (degC()) + " °C";
}


inline double
Temperature::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "k")
		return K();
	else if (unit == "c" || unit == "degc" || unit == "°c")
		return degC();
	else if (unit == "f" || unit == "degf" || unit == "°f")
		return degF();
	else
		throw UnsupportedUnit ("can't convert Temperature to " + unit);
}


/*
 * Global functions
 */


inline constexpr Temperature
operator"" _K (long double kelvins)
{
	return Temperature (static_cast<Temperature::ValueType> (kelvins));
}


inline constexpr Temperature
operator"" _K (unsigned long long kelvins)
{
	return Temperature (static_cast<Temperature::ValueType> (kelvins));
}


inline constexpr Temperature
operator"" _degC (long double degc)
{
	return Temperature (static_cast<Temperature::ValueType> (degc + 273.15));
}


inline constexpr Temperature
operator"" _degC (unsigned long long degc)
{
	return Temperature (static_cast<Temperature::ValueType> (degc + 273.15));
}


inline constexpr Temperature
operator"" _degF (long double degf)
{
	return Temperature (static_cast<Temperature::ValueType> ((degf - 32.0) * 5.0 / 9.0 + 273.15));
}


inline constexpr Temperature
operator"" _degF (unsigned long long degf)
{
	return Temperature (static_cast<Temperature::ValueType> ((degf - 32.0) * 5.0 / 9.0 + 273.15));
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Temperature.
 * Forwards Temperature::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Temperature>: public numeric_limits<SI::Temperature::ValueType>
	{ };

} // namespace std

#endif

