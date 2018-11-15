/* vim:ts=4
 *
 * Copyleft 2012…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__MATH__EULER_ANGLES_H__INCLUDED
#define XEFIS__SUPPORT__MATH__EULER_ANGLES_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/space.h>


namespace xf {

struct EulerAngles: public SpaceVector<si::Angle>
{
	using SpaceVector<si::Angle>::SpaceVector;
	using SpaceVector<si::Angle>::operator=;

	// Ctor
	explicit
	EulerAngles (SpaceVector<si::Angle> const& other):
		SpaceVector<si::Angle> (other)
	{ }

	constexpr auto
	alpha() const noexcept
		{ return (*this)[0]; }

	constexpr auto
	gamma() const noexcept
		{ return (*this)[1]; }

	constexpr auto
	beta() const noexcept
		{ return (*this)[2]; }
};


/**
 * Return a set of Euler angles as difference in rotation between two bases.
 * Order of vector columns in resulting matrix: pitch, roll, yaw.
 * TODO SF1 must be eq to SF2
 */
template<class S, class TF1, class TF2, class SF1, class SF2>
	[[nodiscard]]
	inline EulerAngles
	angle_difference (SpaceMatrix<S, TF1, SF1> const& base_a, SpaceMatrix<S, TF2, SF2> const& base_b)
	{
		using std::atan2;
		using std::sqrt;

		auto const x0 = base_a.column (0); // Heading
		auto const y0 = base_a.column (1); // Pitch
		auto const z0 = base_a.column (2); // Roll
		auto const x3 = base_b.column (0); // Heading
		auto const y3 = base_b.column (1); // Pitch

		// Heading:
		auto const psi = 1_rad * atan2 (static_cast<S> (~x3 * y0), static_cast<S> (~x3 * x0));
		// Pitch:
		auto const theta = 1_rad * atan2 (static_cast<S> (-~x3 * z0), sqrt (square (static_cast<S> (~x3 * x0)) + square (static_cast<S> (~x3 * y0))));
		// Roll:
		auto const y2 = rotation_about (z0, psi) * y0;
		auto const z2 = rotation_about (y2, theta) * z0;
		auto const phi = 1_rad * atan2 (static_cast<S> (~y3 * z2), static_cast<S> (~y3 * y2));

		return { theta, phi, psi };
	}

} // namespace xf

#endif

