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

#ifndef XEFIS__SUPPORT__MATH__SPACE_H__INCLUDED
#define XEFIS__SUPPORT__MATH__SPACE_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Lib:
#include <lib/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/lonlat_radius.h>
#include <xefis/utility/numeric.h>


namespace xf {

// Earth-centered Earth-fixed frame of reference:
struct ECEFFrame;

// Local-tangent-plane frame of reference:
struct NEDFrame;

// Simulated body frame of reference (X points to the front, Y to the right, Z down the body):
// TODO perhaps move to simulation/ or remove
struct AirframeFrame;

// Generic part frame of reference.
struct PartFrame;

// X-Y planar frame of reference, X is along chord in the trailing edge direction, Y points is along lift vector.
// TODO move to simulation/
struct AirfoilSplineFrame;


template<class Scalar = double, class Frame = void>
	using PlaneVector = math::Vector<Scalar, 2, Frame, void>;

template<class Scalar = double, class Frame = void>
	using SpaceVector = math::Vector<Scalar, 3, Frame, void>;

template<class Scalar = double, class TargetFrame = void, class SourceFrame = TargetFrame>
	using PlaneMatrix = math::Matrix<Scalar, 2, 2, TargetFrame, SourceFrame>;

template<class Scalar = double, class TargetFrame = void, class SourceFrame = TargetFrame>
	using SpaceMatrix = math::Matrix<Scalar, 3, 3, TargetFrame, SourceFrame>;

template<class TargetFrame = void, class SourceFrame = TargetFrame>
	using RotationMatrix = SpaceMatrix<double, TargetFrame, SourceFrame>;


/*
 * Polar-cartesian conversions
 */


[[nodiscard]]
inline SpaceVector<si::Length, ECEFFrame>
cartesian (LonLatRadius const& position)
{
	auto const r = si::Length (position.radius()).value();
	auto const wz = std::polar (r, position.lat().in<si::Radian>());
	auto const xy = std::polar (wz.real(), position.lon().in<si::Radian>());

	return {
		si::Length (xy.real()),
		si::Length (xy.imag()),
		si::Length (wz.imag()),
	};
}


[[nodiscard]]
inline LonLatRadius
polar (SpaceVector<si::Length, ECEFFrame> const& vector)
{
	std::complex<double> const xy (vector[0].value(), vector[1].value());
	std::complex<double> const wz (std::abs (xy), vector[2].value());

	return LonLatRadius {
		si::LonLat (1_rad * std::arg (xy), 1_rad * std::arg (wz)),
		si::Length (std::abs (wz))
	};
}


/*
 * Rotations and angle calculations
 */


/**
 * Return rotation matrix along the axis X for given angle.
 */
template<class TF = void, class SF = TF>
	[[nodiscard]]
	constexpr RotationMatrix<TF, SF>
	x_rotation (si::Angle const& angle)
	{
		double const sin_a = sin (angle);
		double const cos_a = cos (angle);

		return {
			1.0,    0.0,    0.0,
			0.0, +cos_a, -sin_a,
			0.0, +sin_a, +cos_a,
		};
	}


/**
 * Return rotation matrix along the axis Y for given angle.
 */
template<class TF = void, class SF = TF>
	[[nodiscard]]
	constexpr RotationMatrix<TF, SF>
	y_rotation (si::Angle const& angle)
	{
		double const sin_a = sin (angle);
		double const cos_a = cos (angle);

		return {
			+cos_a, 0.0, +sin_a,
			   0.0, 1.0,    0.0,
			-sin_a, 0.0, +cos_a,
		};
	}


/**
 * Return rotation matrix along the axis Z for given angle.
 */
template<class TF = void, class SF = TF>
	[[nodiscard]]
	constexpr RotationMatrix<TF, SF>
	z_rotation (si::Angle const& angle)
	{
		double const sin_a = sin (angle);
		double const cos_a = cos (angle);

		return {
			+cos_a, -sin_a, 0.0,
			+sin_a, +cos_a, 0.0,
			   0.0,    0.0, 1.0,
		};
	}


/**
 * Return rotation matrix about the given axis vector for given angle.
 */
template<class TF = void, class SF = TF>
	[[nodiscard]]
	constexpr RotationMatrix<TF, SF>
	rotation_about (SpaceVector<double, TF> const& axis, si::Angle const& angle)
	{
		auto const sin_a = sin (angle);
		auto const cos_a = cos (angle);
		auto const k = 1.0 - cos_a;
		auto const x = axis[0];
		auto const y = axis[1];
		auto const z = axis[2];
		auto const x_sin_a = x * sin_a;
		auto const y_sin_a = y * sin_a;
		auto const z_sin_a = z * sin_a;
		auto const x_y_k = x * y * k;
		auto const x_z_k = x * z * k;
		auto const y_z_k = y * z * k;

		return {
			x * x * k + cos_a, x_y_k - z_sin_a,   x_z_k + y_sin_a,
			x_y_k + z_sin_a,   y * y * k + cos_a, y_z_k - x_sin_a,
			x_z_k - y_sin_a,   y_z_k + x_sin_a,   z * z * k + cos_a,
		};
	}

} // namespace xf

#endif

