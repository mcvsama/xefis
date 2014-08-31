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

#ifndef SI__CAPACITY_H__INCLUDED
#define SI__CAPACITY_H__INCLUDED

// Standard:
#include <cstddef>
#include <limits>

// Local:
#include "linear_value.h"


namespace SI {

class Capacity: public LinearValue<double, Capacity>
{
	friend class LinearValue<double, Capacity>;
	friend constexpr Capacity operator"" _C (long double);
	friend constexpr Capacity operator"" _C (unsigned long long);
	friend constexpr Capacity operator"" _Ah (long double);
	friend constexpr Capacity operator"" _Ah (unsigned long long);
	friend constexpr Capacity operator"" _mAh (long double);
	friend constexpr Capacity operator"" _mAh (unsigned long long);

  protected:
	explicit constexpr
	Capacity (ValueType C) noexcept;

  public:
	constexpr
	Capacity() noexcept = default;

	constexpr
	Capacity (Capacity const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	// Coulombs (Ampere-seconds):
	constexpr ValueType
	C() const noexcept;

	constexpr ValueType
	Ah() const noexcept;

	constexpr ValueType
	mAh() const noexcept;

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


static_assert (std::is_literal_type<Capacity>::value, "Capacity must be a literal type");


/*
 * Global functions
 */


inline constexpr Capacity
operator"" _C (long double C)
{
	return Capacity (static_cast<Capacity::ValueType> (C));
}


inline constexpr Capacity
operator"" _C (unsigned long long C)
{
	return Capacity (static_cast<Capacity::ValueType> (C));
}


inline constexpr Capacity
operator"" _Ah (long double Ah)
{
	return Capacity (static_cast<Capacity::ValueType> (Ah) * 3600.0);
}


inline constexpr Capacity
operator"" _Ah (unsigned long long Ah)
{
	return Capacity (static_cast<Capacity::ValueType> (Ah) * 3600.0);
}


inline constexpr Capacity
operator"" _mAh (long double mAh)
{
	return Capacity (static_cast<Capacity::ValueType> (mAh) * 3.6);
}


inline constexpr Capacity
operator"" _mAh (unsigned long long mAh)
{
	return Capacity (static_cast<Capacity::ValueType> (mAh) * 3.6);
}


/*
 * Capacity implementation
 */


inline constexpr
Capacity::Capacity (ValueType C) noexcept:
	LinearValue (C)
{ }


inline std::vector<std::string> const&
Capacity::supported_units() const
{
	return _supported_units;
}


inline Capacity::ValueType
Capacity::si_units() const noexcept
{
	return C();
}


inline constexpr Capacity::ValueType
Capacity::C() const noexcept
{
	return internal();
}


inline constexpr Capacity::ValueType
Capacity::Ah() const noexcept
{
	return internal() / 3600.0;
}


inline constexpr Capacity::ValueType
Capacity::mAh() const noexcept
{
	return internal() / 3.6;
}


inline void
Capacity::set_si_units (ValueType units)
{
	*this = 1_C * units;
}


inline void
Capacity::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "c")
		*this = p.first * 1_C;
	else if (p.second == "ah")
		*this = p.first * 1_Ah;
	else if (p.second == "mah")
		*this = p.first * 1_mAh;
}


inline std::string
Capacity::stringify() const
{
	return boost::lexical_cast<std::string> (Ah()) + " Ah";
}


inline double
Capacity::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "c")
		return C();
	else if (unit == "ah")
		return Ah();
	else if (unit == "mah")
		return mAh();
	else
		throw UnsupportedUnit ("can't convert Capacity to " + unit);
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Capacity.
 * Forwards Capacity::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Capacity>: public numeric_limits<SI::Capacity::ValueType>
	{ };


template<>
	class is_floating_point<SI::Capacity>: public is_floating_point<SI::Capacity::ValueType>
	{ };

} // namespace std

#endif

