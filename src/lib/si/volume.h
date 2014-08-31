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

#ifndef SI__VOLUME_H__INCLUDED
#define SI__VOLUME_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Volume: public LinearValue<double, Volume>
{
	friend class LinearValue<double, Volume>;
	friend constexpr Volume operator"" _m3 (long double);
	friend constexpr Volume operator"" _m3 (unsigned long long);

  protected:
	explicit constexpr
	Volume (ValueType m3) noexcept;

  public:
	constexpr
	Volume() noexcept = default;

	constexpr
	Volume (Volume const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	m3() const noexcept;

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


static_assert (std::is_literal_type<Volume>::value, "Volume must be a literal type");


/*
 * Global functions
 */


inline constexpr Volume
operator"" _m3 (long double m3)
{
	return Volume (static_cast<Volume::ValueType> (m3));
}


inline constexpr Volume
operator"" _m3 (unsigned long long m3)
{
	return Volume (static_cast<Volume::ValueType> (m3));
}


/*
 * Volume implementation
 */


inline constexpr
Volume::Volume (ValueType m3) noexcept:
	LinearValue (m3)
{ }


inline std::vector<std::string> const&
Volume::supported_units() const
{
	return _supported_units;
}


inline Volume::ValueType
Volume::si_units() const noexcept
{
	return m3();
}


inline constexpr Volume::ValueType
Volume::m3() const noexcept
{
	return internal();
}


inline void
Volume::set_si_units (ValueType units)
{
	*this = 1_m3 * units;
}


inline void
Volume::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "m3" || p.second == "m³")
		*this = p.first * 1_m3;
}


inline std::string
Volume::stringify() const
{
	return boost::lexical_cast<std::string> (m3()) + " m³";
}


inline double
Volume::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "m3" || unit == "m³")
		return m3();
	else
		throw UnsupportedUnit ("can't convert Volume to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Volume.
 * Forwards Volume::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Volume>: public numeric_limits<SI::Volume::ValueType>
	{ };


template<>
	class is_floating_point<SI::Volume>: public is_floating_point<SI::Volume::ValueType>
	{ };

} // namespace std

#endif

