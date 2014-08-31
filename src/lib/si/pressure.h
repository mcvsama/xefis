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

#ifndef SI__PRESSURE_H__INCLUDED
#define SI__PRESSURE_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Pressure: public LinearValue<float, Pressure>
{
	friend class LinearValue<float, Pressure>;
	friend constexpr Pressure operator"" _Pa (long double);
	friend constexpr Pressure operator"" _Pa (unsigned long long);
	friend constexpr Pressure operator"" _hPa (long double);
	friend constexpr Pressure operator"" _hPa (unsigned long long);
	friend constexpr Pressure operator"" _inHg (long double);
	friend constexpr Pressure operator"" _inHg (unsigned long long);
	friend constexpr Pressure operator"" _psi (long double);
	friend constexpr Pressure operator"" _psi (unsigned long long);

  protected:
	/**
	 * Used by the _inhg, _hpa, _psi and similar suffix operators.
	 * To create a Pressure use these operators directly.
	 */
	explicit constexpr
	Pressure (ValueType pa) noexcept;

  public:
	constexpr
	Pressure() noexcept = default;

	constexpr
	Pressure (Pressure const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	Pa() const noexcept;

	constexpr ValueType
	hPa() const noexcept;

	constexpr ValueType
	inHg() const noexcept;

	constexpr ValueType
	psi() const noexcept;

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


static_assert (std::is_literal_type<Pressure>::value, "Pressure must be a literal type");


/*
 * Global functions
 */


inline constexpr Pressure
operator"" _Pa (long double Pa)
{
	return Pressure (static_cast<Pressure::ValueType> (Pa));
}


inline constexpr Pressure
operator"" _Pa (unsigned long long Pa)
{
	return Pressure (static_cast<Pressure::ValueType> (Pa));
}


inline constexpr Pressure
operator"" _hPa (long double hPa)
{
	return Pressure (static_cast<Pressure::ValueType> (hPa) * 0.01);
}


inline constexpr Pressure
operator"" _hPa (unsigned long long hPa)
{
	return Pressure (static_cast<Pressure::ValueType> (hPa) * 0.01);
}


inline constexpr Pressure
operator"" _inHg (long double inHg)
{
	return Pressure (static_cast<Pressure::ValueType> (inHg) * 3386.375258);
}


inline constexpr Pressure
operator"" _inHg (unsigned long long inHg)
{
	return Pressure (static_cast<Pressure::ValueType> (inHg) * 3386.375258);
}


inline constexpr Pressure
operator"" _psi (long double psi)
{
	return Pressure (static_cast<Pressure::ValueType> (psi) * 6894.744825);
}


inline constexpr Pressure
operator"" _psi (unsigned long long psi)
{
	return Pressure (static_cast<Pressure::ValueType> (psi) * 6894.744825);
}


/*
 * Pressure implementation
 */


inline constexpr
Pressure::Pressure (ValueType pa) noexcept:
	LinearValue (pa)
{ }


inline std::vector<std::string> const&
Pressure::supported_units() const
{
	return _supported_units;
}


inline Pressure::ValueType
Pressure::si_units() const noexcept
{
	return Pa();
}


inline constexpr Pressure::ValueType
Pressure::Pa() const noexcept
{
	return internal();
}


inline constexpr Pressure::ValueType
Pressure::hPa() const noexcept
{
	return internal() * 0.01;
}


inline constexpr Pressure::ValueType
Pressure::inHg() const noexcept
{
	return internal() / 3386.375258;
}


inline constexpr Pressure::ValueType
Pressure::psi() const noexcept
{
	return internal() / 6894.744825;
}


inline void
Pressure::set_si_units (ValueType units)
{
	*this = 1_Pa * units;
}


inline void
Pressure::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "pa")
		*this = p.first * 1_Pa;
	else if (p.second == "hpa")
		*this = p.first * 1_hPa;
	else if (p.second == "inhg")
		*this = p.first * 1_inHg;
	else if (p.second == "psi")
		*this = p.first * 1_psi;
}


inline std::string
Pressure::stringify() const
{
	return boost::lexical_cast<std::string> (inHg()) + " inHg";
}


inline double
Pressure::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "pa")
		return Pa();
	else if (unit == "hpa")
		return hPa();
	else if (unit == "inhg")
		return inHg();
	else if (unit == "psi")
		return psi();
	else
		throw UnsupportedUnit ("can't convert Pressure to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Pressure.
 * Forwards Pressure::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Pressure>: public numeric_limits<SI::Pressure::ValueType>
	{ };


template<>
	class is_floating_point<SI::Pressure>: public is_floating_point<SI::Pressure::ValueType>
	{ };

} // namespace std

#endif

