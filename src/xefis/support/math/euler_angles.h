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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>
#include <cmath>


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
 * Angle difference between two rotation matrices.
 * Uses formula Tr(M) = 1 + 2cos(theta).
 * Returns theta.
 */
template<class S, class TF1, class TF2, class SF1, class SF2>
	[[nodiscard]]
	inline si::Angle
	angle_difference (SpaceMatrix<S, TF1, SF1> const& a, SpaceMatrix<S, TF2, SF2> const& b)
	{
		S const unit { 1.0 };
		decltype (unit * unit) const unitsq { 1.0 };

		return 1_rad * std::acos (((trace (~a * b) - unitsq) / 2.0) / unitsq);
	}


/**
 * Return a set of Euler angles as difference in rotation between two bases.
 * Order of vector columns in resulting matrix: pitch, roll, yaw.
 */
template<class S, class TF1, class TF2, class SF1, class SF2>
	[[nodiscard]]
	inline EulerAngles
	euler_angle_difference (SpaceMatrix<S, TF1, SF1> const& base_a, SpaceMatrix<S, TF2, SF2> const& base_b)
	{
		using std::atan2;
		using std::sqrt;

		auto const x0 = base_a.column (0); // Heading
		auto const y0 = base_a.column (1); // Pitch
		auto const z0 = base_a.column (2); // Roll
		auto const x3 = base_b.column (0); // Heading
		auto const y3 = base_b.column (1); // Pitch

		// Heading:
		auto const psi = 1_rad * atan2 ((~x3 * y0).scalar(), (~x3 * x0).scalar());
		// Pitch:
		auto const theta = 1_rad * atan2 ((-~x3 * z0).scalar(), sqrt (square ((~x3 * x0).scalar()) + square ((~x3 * y0).scalar())));
		// Roll:
		auto const y2 = rotation_about (z0, psi) * y0;
		auto const z2 = rotation_about (y2, theta) * z0;
		auto const phi = 1_rad * atan2 ((~y3 * z2).scalar(), (~y3 * y2).scalar());

		return { theta, phi, psi };
	}


template<class S, class SF1, class SF2>
	[[nodiscard]]
	inline EulerAngles
	euler_angles (SpaceMatrix<S, SF1, SF2> const& matrix)
	{
		return euler_angle_difference (SpaceMatrix<S, SF1, SF2> (math::unit), matrix);
	}


/**
 * Return alpha and beta angles required to transform versor x to given vector.
 * Alpha is the X-Y plane angle, beta is X-Z plane angle.
 */
template<class T, class F>
	[[nodiscard]]
	std::array<si::Angle, 2>
	alpha_beta_from_x_to (SpaceVector<T, F> const& vector)
	{
		using std::atan2;

		auto const alpha = 1_rad * atan2 (vector[1], vector[0]);
		auto const r = T (1) * std::sqrt (square (vector[0] / T (1)) + square (vector[1] / T (1)));
		auto const beta = 1_rad * -atan2 (vector[2], r);

		return { alpha, beta };
	}

} // namespace xf

#endif

