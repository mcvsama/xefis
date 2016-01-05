/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef SI__OPERATORS_H__INCLUDED
#define SI__OPERATORS_H__INCLUDED

// Standard:
#include <cstddef>


namespace SI {

inline constexpr Time
operator/ (Length const& length, Speed const& speed)
{
	return 1_h * (length.nmi() / speed.kt());
}


inline constexpr Speed
operator/ (Length const& length, Time const& time)
{
	return 1_kt * (length.nmi() / time.h());
}


inline constexpr Length
operator* (Speed const& speed, Time const& time)
{
	return 1_nmi * (speed.kt() * time.h());
}


inline constexpr Length
operator* (Time const& time, Speed const& speed)
{
	return speed * time;
}


inline constexpr Frequency
operator/ (double value, Time const& time)
{
	return 1_Hz * (value / time.s());
}


inline constexpr Time
operator/ (double value, Frequency const& frequency)
{
	return 1_s * (value / frequency.Hz());
}


inline constexpr Frequency
operator/ (Angle const& angle, Time const& time)
{
	return 1_Hz * (angle.rad() / (2.0 * M_PI * time.s()));
}


inline constexpr Acceleration
operator/ (Speed const& speed, Time const& time)
{
	return 1_mps2 * (speed.mps() / time.s());
}


inline constexpr Acceleration
operator* (Frequency const& frequency, Speed const& speed)
{
	return 1_mps2 * (frequency.Hz() * speed.mps());
}


inline constexpr Acceleration
operator* (Speed const& speed, Frequency const& frequency)
{
	return frequency * speed;
}


inline constexpr Length
operator/ (Speed const& speed, Frequency const& frequency)
{
	return 1_m * (speed.mps() / frequency.Hz());
}


inline constexpr Angle
operator* (Time const& time, Frequency const& frequency)
{
	return 1_rad * (frequency.Hz() * 2.0 * M_PI * time.s());
}


inline constexpr Angle
operator* (Frequency const& frequency, Time const& time)
{
	return time * frequency;
}


inline constexpr Force
operator* (Weight const& mass, Acceleration const& acceleration)
{
	return 1_N * (mass.kg() * acceleration.mps2());
}


inline constexpr Force
operator* (Acceleration const& acceleration, Weight const& mass)
{
	return mass * acceleration;
}


inline constexpr Pressure
operator/ (Force const& force, Area const& area)
{
	return 1_Pa * (force.N() / area.m2());
}


inline constexpr Energy
operator* (Force const& force, Length const& length)
{
	return 1_J * (force.N() * length.m());
}


inline constexpr Energy
operator* (Length const& length, Force const& force)
{
	return force * length;
}


inline constexpr Power
operator/ (Energy const& energy, Time const& time)
{
	return 1_W * (energy.J() / time.s());
}


inline constexpr Power
operator* (Force const& force, Speed const& speed)
{
	return 1_W * (force.N() * speed.mps());
}


inline constexpr Power
operator* (Speed const& speed, Force const& force)
{
	return force * speed;
}

} // namespace SI

#endif

