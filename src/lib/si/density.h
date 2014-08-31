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

#ifndef SI__DENSITY_H__INCLUDED
#define SI__DENSITY_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Density: public LinearValue<double, Density>
{
	friend class LinearValue<double, Density>;
	friend constexpr Density operator"" _kgpm3 (long double);
	friend constexpr Density operator"" _kgpm3 (unsigned long long);

  protected:
	explicit constexpr
	Density (ValueType kgpm3) noexcept;

  public:
	constexpr
	Density() noexcept = default;

	constexpr
	Density (Density const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	kgpm3() const noexcept;

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


static_assert (std::is_literal_type<Density>::value, "Density must be a literal type");


/*
 * Global functions
 */


inline constexpr Density
operator"" _kgpm3 (long double kgpm3)
{
	return Density (static_cast<Density::ValueType> (kgpm3));
}


inline constexpr Density
operator"" _kgpm3 (unsigned long long kgpm3)
{
	return Density (static_cast<Density::ValueType> (kgpm3));
}


/*
 * Density implementation
 */


inline constexpr
Density::Density (ValueType kgpm3) noexcept:
	LinearValue (kgpm3)
{ }


inline std::vector<std::string> const&
Density::supported_units() const
{
	return _supported_units;
}


inline Density::ValueType
Density::si_units() const noexcept
{
	return kgpm3();
}


inline constexpr Density::ValueType
Density::kgpm3() const noexcept
{
	return internal();
}


inline void
Density::set_si_units (ValueType units)
{
	*this = 1_kgpm3 * units;
}


inline void
Density::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "kgpm3")
		*this = p.first * 1_kgpm3;
}


inline std::string
Density::stringify() const
{
	return boost::lexical_cast<std::string> (kgpm3()) + " kg/m³";
}


inline double
Density::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "kgpm3" || unit == "kg/m3" || unit == "kg/m³")
		return kgpm3();
	else
		throw UnsupportedUnit ("can't convert Density to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Density.
 * Forwards Density::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Density>: public numeric_limits<SI::Density::ValueType>
	{ };


template<>
	class is_floating_point<SI::Density>: public is_floating_point<SI::Density::ValueType>
	{ };

} // namespace std

#endif

