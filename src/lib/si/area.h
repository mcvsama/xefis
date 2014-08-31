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

#ifndef SI__AREA_H__INCLUDED
#define SI__AREA_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Area: public LinearValue<double, Area>
{
	friend class LinearValue<double, Area>;
	friend constexpr Area operator"" _m2 (long double);
	friend constexpr Area operator"" _m2 (unsigned long long);

  protected:
	explicit constexpr
	Area (ValueType m2) noexcept;

  public:
	constexpr
	Area() noexcept = default;

	constexpr
	Area (Area const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	m2() const noexcept;

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


static_assert (std::is_literal_type<Area>::value, "Area must be a literal type");


/*
 * Global functions
 */


inline constexpr Area
operator"" _m2 (long double m2)
{
	return Area (static_cast<Area::ValueType> (m2));
}


inline constexpr Area
operator"" _m2 (unsigned long long m2)
{
	return Area (static_cast<Area::ValueType> (m2));
}


/*
 * Area implementation
 */


inline constexpr
Area::Area (ValueType m2) noexcept:
	LinearValue (m2)
{ }


inline std::vector<std::string> const&
Area::supported_units() const
{
	return _supported_units;
}


inline Area::ValueType
Area::si_units() const noexcept
{
	return m2();
}


inline constexpr Area::ValueType
Area::m2() const noexcept
{
	return internal();
}


inline void
Area::set_si_units (ValueType units)
{
	*this = 1_m2 * units;
}


inline void
Area::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "m2" || p.second == "m²")
		*this = p.first * 1_m2;
}


inline std::string
Area::stringify() const
{
	return boost::lexical_cast<std::string> (m2()) + " m²";
}


inline double
Area::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "m2" || unit == "m²")
		return m2();
	else
		throw UnsupportedUnit ("can't convert Area to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Area.
 * Forwards Area::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Area>: public numeric_limits<SI::Area::ValueType>
	{ };


template<>
	class is_floating_point<SI::Area>: public is_floating_point<SI::Area::ValueType>
	{ };

} // namespace std

#endif

