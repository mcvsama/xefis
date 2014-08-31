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
	friend constexpr Weight operator"" _lb (long double);
	friend constexpr Weight operator"" _lb (unsigned long long);

  protected:
	explicit constexpr
	Weight (ValueType kg) noexcept;

  public:
	constexpr
	Weight() noexcept = default;

	constexpr
	Weight (Weight const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	gr() const noexcept;

	constexpr ValueType
	kg() const noexcept;

	constexpr ValueType
	lb() const noexcept;

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


static_assert (std::is_literal_type<Weight>::value, "Weight must be a literal type");


/*
 * Global functions
 */


inline constexpr Weight
operator"" _gr (long double gr)
{
	return Weight (static_cast<Weight::ValueType> (gr) / 1000.0);
}


inline constexpr Weight
operator"" _gr (unsigned long long gr)
{
	return Weight (static_cast<Weight::ValueType> (gr) / 1000.0);
}


inline constexpr Weight
operator"" _kg (long double kg)
{
	return Weight (static_cast<Weight::ValueType> (kg));
}


inline constexpr Weight
operator"" _kg (unsigned long long kg)
{
	return Weight (static_cast<Weight::ValueType> (kg));
}


inline constexpr Weight
operator"" _lb (long double lb)
{
	return Weight (static_cast<Weight::ValueType> (lb) * 0.453592);
}


inline constexpr Weight
operator"" _lb (unsigned long long lb)
{
	return Weight (static_cast<Weight::ValueType> (lb) * 0.453592);
}


/*
 * Weight implementation
 */


inline constexpr
Weight::Weight (ValueType kg) noexcept:
	LinearValue (kg)
{ }


inline std::vector<std::string> const&
Weight::supported_units() const
{
	return _supported_units;
}


inline Weight::ValueType
Weight::si_units() const noexcept
{
	return kg();
}


inline constexpr Weight::ValueType
Weight::gr() const noexcept
{
	return internal() * 1000.0;
}


inline constexpr Weight::ValueType
Weight::kg() const noexcept
{
	return internal();
}


inline constexpr Weight::ValueType
Weight::lb() const noexcept
{
	return internal() / 0.453592;
}


inline void
Weight::set_si_units (ValueType units)
{
	*this = 1_kg * units;
}


inline void
Weight::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "gr" || p.second == "gram")
		*this = p.first * 1_gr;
	else if (p.second == "kg")
		*this = p.first * 1_kg;
	else if (p.second == "lb")
		*this = p.first * 1_lb;
}


inline std::string
Weight::stringify() const
{
	return boost::lexical_cast<std::string> (kg()) + " kg";
}


inline double
Weight::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "gr" || unit == "gram")
		return gr();
	else if (unit == "kg")
		return kg();
	else if (unit == "lb")
		return lb();
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

