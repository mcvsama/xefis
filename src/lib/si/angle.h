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

#ifndef SI__ANGLE_H__INCLUDED
#define SI__ANGLE_H__INCLUDED

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

class Angle: public LinearValue<double, Angle>
{
	friend class LinearValue<double, Angle>;
	friend constexpr Angle operator"" _rad (long double);
	friend constexpr Angle operator"" _rad (unsigned long long);
	friend constexpr Angle operator"" _deg (long double);
	friend constexpr Angle operator"" _deg (unsigned long long);

  protected:
	/**
	 * Used by the _rad and _deg suffix operators.
	 * To create an Angle use these operators directly.
	 */
	explicit constexpr
	Angle (ValueType radians) noexcept;

  public:
	constexpr
	Angle() noexcept = default;

	constexpr
	Angle (Angle const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	rad() const noexcept;

	constexpr ValueType
	deg() const noexcept;

	void
	set_si_units (ValueType) override;

	void
	parse (std::string const&) override;

	std::string
	stringify() const override;

	double
	floatize (std::string unit) const override;

	/**
	 * Convert to Degrees/Minutes/Seconds format.
	 * Uses +/- notation on degrees.
	 */
	std::string
	to_dms (bool three_digits = false) const;

	/**
	 * Like to_dms(), but uses N/S suffix to denote
	 * latitude.
	 */
	std::string
	to_latitude_dms() const;

	/**
	 * Like to_dms(), but uses E/W suffix to denote
	 * longitude.
	 */
	std::string
	to_longitude_dms() const;

	/**
	 * Mean value for two angles on a circle.
	 */
	static Angle
	mean (Angle lhs, Angle rhs);

  private:
	static std::vector<std::string> _supported_units;
};


static_assert (std::is_literal_type<Angle>::value, "Angle must be a literal type");


/*
 * Global functions
 */


inline constexpr Angle
operator"" _rad (long double radians)
{
	return Angle (static_cast<Angle::ValueType> (radians));
}


inline constexpr Angle
operator"" _rad (unsigned long long radians)
{
	return Angle (static_cast<Angle::ValueType> (radians));
}


inline constexpr Angle
operator"" _deg (long double degrees)
{
	return Angle (static_cast<Angle::ValueType> (degrees * M_PI / 180.0));
}


inline constexpr Angle
operator"" _deg (unsigned long long degrees)
{
	return Angle (static_cast<Angle::ValueType> (degrees) * M_PI / 180.0);
}


/*
 * Angle implementation
 */


inline constexpr
Angle::Angle (ValueType radians) noexcept:
	LinearValue (radians)
{ }


inline std::vector<std::string> const&
Angle::supported_units() const
{
	return _supported_units;
}


inline Angle::ValueType
Angle::si_units() const noexcept
{
	return rad();
}


inline constexpr Angle::ValueType
Angle::rad() const noexcept
{
	return internal();
}


inline constexpr Angle::ValueType
Angle::deg() const noexcept
{
	return internal() * 180.0 / M_PI;
}


inline void
Angle::set_si_units (ValueType units)
{
	*this = 1_rad * units;
}


inline void
Angle::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "deg" || p.second == "°")
		*this = p.first * 1_deg;
	else if (p.second == "rad")
		*this = p.first * 1_rad;
}


inline std::string
Angle::stringify() const
{
	return boost::lexical_cast<std::string> (deg()) + " °";
}


inline double
Angle::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "deg" || unit == "°")
		return deg();
	else if (unit == "rad")
		return rad();
	else
		throw UnsupportedUnit ("can't convert Angle to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Angle.
 * Forwards Angle::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Angle>: public numeric_limits<SI::Angle::ValueType>
	{ };


template<>
	class is_floating_point<SI::Angle>: public is_floating_point<SI::Angle::ValueType>
	{ };


#define FORWARD_ANGLE_TO_STD_FUNCTION_1(function_name)				\
	inline SI::Angle::ValueType function_name (SI::Angle const& a)	\
	{																\
		return function_name (a.rad());								\
	}

FORWARD_ANGLE_TO_STD_FUNCTION_1 (sin)
FORWARD_ANGLE_TO_STD_FUNCTION_1 (cos)
FORWARD_ANGLE_TO_STD_FUNCTION_1 (tan)

#undef FORWARD_ANGLE_TO_STD_FUNCTION_1

} // namespace std

#endif

