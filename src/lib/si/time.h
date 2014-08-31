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

#ifndef SI__TIME_H__INCLUDED
#define SI__TIME_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <limits>

// System:
#include <sys/time.h>

// Local:
#include "linear_value.h"


namespace SI {

class Time: public LinearValue<double, Time>
{
	friend class LinearValue<double, Time>;
	friend constexpr Time operator"" _ns (long double);
	friend constexpr Time operator"" _ns (unsigned long long);
	friend constexpr Time operator"" _us (long double);
	friend constexpr Time operator"" _us (unsigned long long);
	friend constexpr Time operator"" _ms (long double);
	friend constexpr Time operator"" _ms (unsigned long long);
	friend constexpr Time operator"" _s (long double);
	friend constexpr Time operator"" _s (unsigned long long);
	friend constexpr Time operator"" _min (long double);
	friend constexpr Time operator"" _min (unsigned long long);
	friend constexpr Time operator"" _h (long double);
	friend constexpr Time operator"" _h (unsigned long long);

  protected:
	/**
	 * Used by the suffix operators.
	 */
	explicit constexpr
	Time (ValueType seconds) noexcept;

  public:
	constexpr
	Time() noexcept = default;

	constexpr
	Time (Time const&) noexcept = default;

	std::vector<std::string> const&
	supported_units() const override;

	ValueType
	si_units() const noexcept override;

	constexpr ValueType
	ns() const noexcept;

	constexpr ValueType
	us() const noexcept;

	constexpr ValueType
	ms() const noexcept;

	constexpr ValueType
	s() const noexcept;

	constexpr ValueType
	min() const noexcept;

	constexpr ValueType
	h() const noexcept;

	void
	set_si_units (ValueType) override;

	void
	parse (std::string const&) override;

	std::string
	stringify() const override;

	double
	floatize (std::string unit) const override;

  public:
	static Time
	now() noexcept;

	static Time
	epoch() noexcept;

	static Time
	measure (std::function<void()> callback) noexcept;

  private:
	static std::vector<std::string> _supported_units;
};


static_assert (std::is_literal_type<Time>::value, "Time must be a literal type");


/*
 * Global functions
 */


inline constexpr Time
operator"" _ns (long double ns)
{
	return Time (static_cast<Time::ValueType> (ns / 1e9));
}


inline constexpr Time
operator"" _ns (unsigned long long ns)
{
	return Time (static_cast<Time::ValueType> (ns / 1e9));
}


inline constexpr Time
operator"" _us (long double us)
{
	return Time (static_cast<Time::ValueType> (us / 1e6));
}


inline constexpr Time
operator"" _us (unsigned long long us)
{
	return Time (static_cast<Time::ValueType> (us / 1e6));
}


inline constexpr Time
operator"" _ms (long double ms)
{
	return Time (static_cast<Time::ValueType> (ms / 1e3));
}


inline constexpr Time
operator"" _ms (unsigned long long ms)
{
	return Time (static_cast<Time::ValueType> (ms / 1e3));
}


inline constexpr Time
operator"" _s (long double seconds)
{
	return Time (static_cast<Time::ValueType> (seconds));
}


inline constexpr Time
operator"" _s (unsigned long long seconds)
{
	return Time (static_cast<Time::ValueType> (seconds));
}


inline constexpr Time
operator"" _min (long double m)
{
	return Time (static_cast<Time::ValueType> (m * 60.0));
}


inline constexpr Time
operator"" _min (unsigned long long m)
{
	return Time (static_cast<Time::ValueType> (m * 60.0));
}


inline constexpr Time
operator"" _h (long double h)
{
	return Time (static_cast<Time::ValueType> (h * 3600.0));
}


inline constexpr Time
operator"" _h (unsigned long long h)
{
	return Time (static_cast<Time::ValueType> (h * 3600.0));
}


/*
 * Time implementation
 */


inline constexpr
Time::Time (ValueType seconds) noexcept:
	LinearValue (seconds)
{ }


inline std::vector<std::string> const&
Time::supported_units() const
{
	return _supported_units;
}


inline Time::ValueType
Time::si_units() const noexcept
{
	return s();
}


inline constexpr Time::ValueType
Time::ns() const noexcept
{
	return internal() * 1e9;
}


inline constexpr Time::ValueType
Time::us() const noexcept
{
	return internal() * 1e6;
}


inline constexpr Time::ValueType
Time::ms() const noexcept
{
	return internal() * 1e3;
}


inline constexpr Time::ValueType
Time::s() const noexcept
{
	return internal();
}


inline constexpr Time::ValueType
Time::min() const noexcept
{
	return internal() / 60.0;
}


inline constexpr Time::ValueType
Time::h() const noexcept
{
	return internal() / 3600.0;
}


inline void
Time::set_si_units (ValueType units)
{
	*this = 1_s * units;
}


inline void
Time::parse (std::string const& str)
{
	auto p = generic_parse (str);

	if (p.second == "ns")
		*this = p.first * 1_ns;
	else if (p.second == "us")
		*this = p.first * 1_us;
	else if (p.second == "ms")
		*this = p.first * 1_ms;
	else if (p.second == "s")
		*this = p.first * 1_s;
	else if (p.second == "min")
		*this = p.first * 1_min;
	else if (p.second == "h")
		*this = p.first * 1_h;
}


inline std::string
Time::stringify() const
{
	return boost::lexical_cast<std::string> (s()) + " s";
}


inline double
Time::floatize (std::string unit) const
{
	boost::to_lower (unit);

	if (unit == "ns")
		return ns();
	else if (unit == "us")
		return us();
	else if (unit == "ms")
		return ms();
	else if (unit == "s")
		return s();
	else if (unit == "min")
		return min();
	else if (unit == "h")
		return h();
	else
		throw UnsupportedUnit ("can't convert Time to " + unit);
}


inline Time
Time::now() noexcept
{
	struct timeval tv;
	::gettimeofday (&tv, 0);
	return 1_us * static_cast<ValueType> (tv.tv_sec * 1000000ull + tv.tv_usec);
}


inline Time
Time::epoch() noexcept
{
	Time t;
	t.internal() = 0.0;
	return t;
}


inline Time
Time::measure (std::function<void()> callback) noexcept
{
	Time t = now();
	callback();
	return now() - t;
}

} // namespace SI


namespace std {

/**
 * Numeric limits for class Time.
 * Forwards Time::ValueType as parameter to std::numeric_limits.
 */
template<>
	class numeric_limits<SI::Time>: public numeric_limits<SI::Time::ValueType>
	{ };


template<>
	class is_floating_point<SI::Time>: public is_floating_point<SI::Time::ValueType>
	{ };

} // namespace std

#endif

