/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__UTILITY__TIMESTAMP_H__INCLUDED
#define XEFIS__UTILITY__TIMESTAMP_H__INCLUDED

// Standard:
#include <cstddef>

// System:
#include <sys/time.h>

// Haruhi:
#include <xefis/config/all.h>


namespace Xefis {

class Timestamp
{
  public:
	/**
	 * Construct "0" timestamp.
	 */
	constexpr
	Timestamp() noexcept;

	constexpr bool
	operator< (Timestamp const& other) const noexcept;

	constexpr bool
	operator> (Timestamp const& other) const noexcept;

	constexpr Timestamp
	operator-() const noexcept;

	Timestamp&
	operator+= (Timestamp const& other) noexcept;

	Timestamp&
	operator-= (Timestamp const& other) noexcept;

	constexpr Timestamp
	operator+ (Timestamp const& other) const noexcept;

	constexpr Timestamp
	operator- (Timestamp const& other) const noexcept;

	/**
	 * Return UNIX time for given timestamp measured in µs.
	 */
	constexpr int64_t
	microseconds() const noexcept;

	/**
	 * Return time in seconds.
	 */
	constexpr double
	seconds() const noexcept;

	/**
	 * Assign current time to the timestamp.
	 */
	void
	touch() noexcept;

	/**
	 * Assign given value of Epoch seconds to the timestamp.
	 */
	void
	set_epoch (int64_t epoch) noexcept;

	/**
	 * Assign given value of Epoch microseconds to the timestamp.
	 */
	void
	set_epoch_microseconds (int64_t epoch_microseconds) noexcept;

  public:
	static Timestamp
	now() noexcept;

	// FIXME: GCC bug <http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53473> - remove "(false)" when this is fixed.
	static constexpr Timestamp
	from_epoch (int64_t epoch) noexcept (false);

	// FIXME: GCC bug <http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53473> - remove "(false)" when this is fixed.
	static constexpr Timestamp
	from_epoch_microseconds (int64_t epoch_us) noexcept (false);

  private:
	// Ctor
	constexpr explicit
	Timestamp (int64_t epoch_microseconds) noexcept;

  private:
	int64_t _epoch_us;
};


inline constexpr
Timestamp::Timestamp() noexcept:
	_epoch_us (0)
{ }


inline constexpr bool
Timestamp::operator< (Timestamp const& other) const noexcept
{
	return _epoch_us < other._epoch_us;
}


inline constexpr bool
Timestamp::operator> (Timestamp const& other) const noexcept
{
	return other < *this;
}


inline constexpr Timestamp
Timestamp::operator-() const noexcept
{
	return Timestamp (-_epoch_us);
}


inline Timestamp&
Timestamp::operator+= (Timestamp const& other) noexcept
{
	_epoch_us += other._epoch_us;
	return *this;
}


inline Timestamp&
Timestamp::operator-= (Timestamp const& other) noexcept
{
	_epoch_us -= other._epoch_us;
	return *this;
}


inline constexpr Timestamp
Timestamp::operator+ (Timestamp const& other) const noexcept
{
	return Timestamp (_epoch_us + other._epoch_us);
}


inline constexpr Timestamp
Timestamp::operator- (Timestamp const& other) const noexcept
{
	return Timestamp (_epoch_us - other._epoch_us);
}


inline constexpr int64_t
Timestamp::microseconds() const noexcept
{
	return _epoch_us;
}


inline constexpr double
Timestamp::seconds() const noexcept
{
	return _epoch_us / 1e6;
}


inline void
Timestamp::touch() noexcept
{
	struct timeval t;
	::gettimeofday (&t, 0);
	_epoch_us = t.tv_sec * 1000000ull + t.tv_usec;
}


inline void
Timestamp::set_epoch (int64_t epoch) noexcept
{
	_epoch_us = epoch * 1000000ull;
}


inline void
Timestamp::set_epoch_microseconds (int64_t epoch_microseconds) noexcept
{
	_epoch_us = epoch_microseconds;
}


inline Timestamp
Timestamp::now() noexcept
{
	Timestamp t;
	t.touch();
	return t;
}


// FIXME: GCC bug <http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53473> - remove "(false)" when this is fixed.
inline constexpr Timestamp
Timestamp::from_epoch (int64_t epoch) noexcept (false)
{
	return Timestamp (epoch * 1000000ull);
}


// FIXME: GCC bug <http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53473> - remove "(false)" when this is fixed.
inline constexpr Timestamp
Timestamp::from_epoch_microseconds (int64_t epoch_us) noexcept (false)
{
	return Timestamp (epoch_us);
}


inline constexpr
Timestamp::Timestamp (int64_t epoch_us) noexcept:
	_epoch_us (epoch_us)
{ }

} // namespace Xefis

#endif

