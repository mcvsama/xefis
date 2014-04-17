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

#ifndef SI__WEIGHT_H__INCLUDED
#define SI__WEIGHT_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Weight: public LinearValue<double, Weight>
{
	friend class LinearValue<double, Weight>;
	friend constexpr Weight operator"" _gr (long double);
	friend constexpr Weight operator"" _gr (unsigned long long);
	friend constexpr Weight operator"" _kg (long double);
	friend constexpr Weight operator"" _kg (unsigned long long);

  protected:
	explicit constexpr
	Weight (ValueType psi);

  public:
	constexpr
	Weight() = default;

	constexpr
	Weight (Weight const&) = default;

	std::vector<std::string> const&
	supported_units() const override;

	constexpr ValueType
	gr() const noexcept;

	constexpr ValueType
	kg() const noexcept;

	void
	parse (std::string const&) override;

	std::string
	stringify() const override;

	double
	floatize (std::string unit) const override;

  private:
	static std::vector<std::string> _supported_units;
};


/*
 * Global functions
 */


inline constexpr Weight
operator"" _gr (long double gr)
{
	return Weight (static_cast<Weight::ValueType> (gr));
}


inline constexpr Weight
operator"" _gr (unsigned long long gr)
{
	return Weight (static_cast<Weight::ValueType> (gr));
}


inline constexpr Weight
operator"" _kg (long double kg)
{
	return Weight (static_cast<Weight::ValueType> (kg) * 1000.0);
}


inline constexpr Weight
operator"" _kg (unsigned long long kg)
{
	return Weight (static_cast<Weight::ValueType> (kg) * 1000.0);
}


/*
 * Weight implementation
 */


inline constexpr
Weight::Weight (ValueType m):
	LinearValue (m)
{ }


inline std::vector<std::string> const&
Weight::supported_units() const
{
	return _supported_units;
}


inline constexpr Weight::ValueType
Weight::gr() const noexcept
{
	return internal();
}


inline constexpr Weight::ValueType
Weight::kg() const noexcept
{
	return internal() * 0.001;
}


inline void
Weight::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "gr" || p.second == "gram")
		*this = p.first * 1_gr;
	else if (p.second == "kg")
		*this = p.first * 1_kg;
}


inline std::string
Weight::stringify() const
{
	return boost::lexical_cast<std::string> (gr()) + " gr";
}


inline double
Weight::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "gr")
		return gr();
	else if (unit == "kg")
		return kg();
	else
		throw UnsupportedUnit ("can't convert Weight to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Weight.
 * Forwards Weight::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Weight>: public numeric_limits<SI::Weight::ValueType>
	{ };


template<>
	class is_floating_point<SI::Weight>: public is_floating_point<SI::Weight::ValueType>
	{ };

} // namespace std

#endif

