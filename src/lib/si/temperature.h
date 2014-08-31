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
	friend constexpr Temperature operator"" _Ra (long double);
	friend constexpr Temperature operator"" _Ra (unsigned long long);

  protected:
	/**
	 * Used by the suffix operators.
	 */
	explicit constexpr
	Temperature (ValueType kelvins) noexcept;

  public:
	constexpr
	Temperature() noexcept = default;

	constexpr
	Temperature (Temperature const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	/**
	 * Return number of Kelvins.
	 */
	constexpr ValueType
	K() const noexcept;

	/**
	 * Return number of Rankines.
	 */
	constexpr ValueType
	Ra() const noexcept;

	/**
	 * Convert to Celsius degrees.
	 */
	constexpr ValueType
	degC() const noexcept;

	/**
	 * Convert to Fahrenheit degrees.
	 */
	constexpr ValueType
	degF() const noexcept;

	void
	set_si_units (ValueType) override;

	void
	parse (std::string const&) override;

	std::string
	stringify() const override;

	double
	floatize (std::string unit) const override;

	/**
	 * Return Temperature from given number of Celsius degrees.
	 */
	static constexpr Temperature
	from_degC (double celsius);

	/**
	 * Return Temperature from given number of Fahrenheit degrees.
	 */
	static constexpr Temperature
	from_degF (double fahrenheit);

  private:
	static std::vector<std::string> _supported_units;
};


static_assert (std::is_literal_type<Temperature>::value, "Temperature must be a literal type");


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
operator"" _Ra (long double rankines)
{
	return Temperature (static_cast<Temperature::ValueType> (rankines) / 1.8);
}


inline constexpr Temperature
operator"" _Ra (unsigned long long rankines)
{
	return Temperature (static_cast<Temperature::ValueType> (rankines) / 1.8);
}


/*
 * Temperature implementation
 */


inline constexpr
Temperature::Temperature (ValueType kelvins) noexcept:
	LinearValue (kelvins)
{ }


inline std::vector<std::string> const&
Temperature::supported_units() const
{
	return _supported_units;
}


inline Temperature::ValueType
Temperature::si_units() const noexcept
{
	return K();
}


inline constexpr Temperature::ValueType
Temperature::K() const noexcept
{
	return internal();
}


inline constexpr Temperature::ValueType
Temperature::Ra() const noexcept
{
	return K() * 1.8;
}


inline constexpr Temperature::ValueType
Temperature::degC() const noexcept
{
	return internal() - 273.15;
}


inline constexpr Temperature::ValueType
Temperature::degF() const noexcept
{
	return internal() * 1.8 - 459.67;
}


inline void
Temperature::set_si_units (ValueType units)
{
	*this = 1_K * units;
}


inline void
Temperature::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "k")
		*this = p.first * 1_K;
	else if (p.second == "ra")
		*this = p.first * 1_Ra;
	else if (p.second == "c" || p.second == "degc" || p.second == "°c")
		*this = Temperature::from_degC (p.first);
	else if (p.second == "f" || p.second == "degf" || p.second == "°f")
		*this = Temperature::from_degF (p.first);
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
	else if (unit == "r" || unit == "ra")
		return Ra();
	else if (unit == "c" || unit == "degc" || unit == "°c")
		return degC();
	else if (unit == "f" || unit == "degf" || unit == "°f")
		return degF();
	else
		throw UnsupportedUnit ("can't convert Temperature to " + unit);
}


inline constexpr Temperature
Temperature::from_degC (double celsius)
{
	return Temperature (celsius + 273.15);
}


inline constexpr Temperature
Temperature::from_degF (double fahrenheit)
{
	return Temperature (fahrenheit + 459.67) / 1.8;
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


template<>
	class is_floating_point<SI::Temperature>: public is_floating_point<SI::Temperature::ValueType>
	{ };

} // namespace std

#endif

