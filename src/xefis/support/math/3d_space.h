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

#ifndef XEFIS__SUPPORT__MATH__3D_SPACE_H__INCLUDED
#define XEFIS__SUPPORT__MATH__3D_SPACE_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <lib/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/lonlat_radius.h>
#include <xefis/utility/numeric.h>


namespace xf {

template<class Value = double>
	using SpaceVector = math::Vector<Value, 3>;

template<class Value = double>
	using SpaceMatrix = math::Matrix<Value, 3, 3>;

using SpaceQuaternion = math::Quaternion<double>;


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
	{
		return operator[] (0);
	}

	constexpr auto
	beta() const noexcept
	{
		return operator[] (1);
	}

	constexpr auto
	gamma() const noexcept
	{
		return operator[] (2);
	}
};


/**
 * NED's (aka local tangent plane's) X, Y, Z vectors in ECEF frame.
 */
struct NorthEastDown: public SpaceMatrix<>
{
	using SpaceMatrix<>::SpaceMatrix;
	using SpaceMatrix<>::operator=;

	// Ctor
	explicit
	NorthEastDown (SpaceMatrix<> const& other):
		SpaceMatrix<> (other)
	{ }

	constexpr auto
	north() const noexcept
	{
		return column (0);
	}

	constexpr auto
	east() const noexcept
	{
		return column (1);
	}

	constexpr auto
	down() const noexcept
	{
		return column (2);
	}
};


/**
 * Pitch, yaw, roll in NED frame.
 */
struct TaitBryanAngles: public SpaceVector<si::Angle>
{
	using SpaceVector<si::Angle>::SpaceVector;
	using SpaceVector<si::Angle>::operator=;

	// Ctor
	explicit
	TaitBryanAngles (SpaceVector<si::Angle> const& other):
		SpaceVector<si::Angle> (other)
	{ }

	constexpr auto
	pitch() const noexcept
	{
		return operator[] (0);
	}

	constexpr auto
	roll() const noexcept
	{
		return operator[] (1);
	}

	constexpr auto
	yaw() const noexcept
	{
		return operator[] (2);
	}
};


static inline NorthEastDown const EquatorPrimeMeridian = {
//  N   E   D
	0,  0, -1,  // x
	0,  1,  0,  // y
	1,  0,  0,  // z
};


NorthEastDown
ned_matrix (si::LonLat const& position);


inline SpaceVector<si::Length>
cartesian (LonLatRadius const& position)
{
	auto const r = si::Length (position.radius()).quantity();
	auto const wz = std::polar (r, position.lat().in<si::Radian>());
	auto const xy = std::polar (wz.real(), position.lon().in<si::Radian>());

	return {
		si::Length (xy.real()),
		si::Length (xy.imag()),
		si::Length (wz.imag()),
	};
}


inline LonLatRadius
polar (SpaceVector<si::Length> const& vector)
{
	std::complex<double> const xy (vector[0].quantity(), vector[1].quantity());
	std::complex<double> const wz (std::abs (xy), vector[2].quantity());

	return LonLatRadius {
		si::LonLat (1_rad * std::arg (xy), 1_rad * std::arg (wz)),
		si::Length (std::abs (wz))
	};
}


/**
 * Return rotation matrix along the axis X for given angle.
 */
constexpr SpaceMatrix<>
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
constexpr SpaceMatrix<>
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
constexpr SpaceMatrix<>
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
template<class Value>
	constexpr SpaceMatrix<>
	rotation_about (SpaceVector<Value> axis, si::Angle const& angle)
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

		return SpaceMatrix<> {
			x * x * k + cos_a, x_y_k - z_sin_a,   x_z_k + y_sin_a,
			x_y_k + z_sin_a,   y * y * k + cos_a, y_z_k - x_sin_a,
			x_z_k - y_sin_a,   y_z_k + x_sin_a,   z * z * k + cos_a,
		};
	}


/**
 * Return a set of Euler angles as difference in rotation between two bases.
 * Order of vector columns in matrices: pitch, roll, yaw.
 */
template<class Value>
	inline EulerAngles
	angle_difference (SpaceMatrix<Value> const& base_a, SpaceMatrix<Value> const& base_b)
	{
		using std::atan2;
		using std::sqrt;

		auto const x0 = base_a.column (0); // Heading
		auto const y0 = base_a.column (1); // Pitch
		auto const z0 = base_a.column (2); // Roll
		auto const x3 = base_b.column (0); // Heading
		auto const y3 = base_b.column (1); // Pitch

		// Heading:
		auto const psi = 1_rad * atan2 (static_cast<Value> (~x3 * y0), static_cast<Value> (~x3 * x0));
		// Pitch:
		auto const theta = 1_rad * atan2 (static_cast<Value> (-~x3 * z0), sqrt ((square (~x3 * x0) + square (~x3 * y0))[0]));

		auto const y2 = rotation_about (z0, psi) * y0;
		auto const z2 = rotation_about (y2, theta) * z0;
		// Roll:
		auto const phi = 1_rad * atan2 (static_cast<Value> (~y3 * z2), static_cast<Value> (~y3 * y2));

		return { theta, phi, psi };
	}


inline TaitBryanAngles
tait_bryan_angles (SpaceMatrix<> const& ecef_orientation, si::LonLat const& position)
{
	return TaitBryanAngles (static_cast<SpaceVector<si::Angle>> (angle_difference (ned_matrix (position), ecef_orientation)));
}


inline TaitBryanAngles
tait_bryan_angles (SpaceMatrix<> const& ecef_orientation, SpaceVector<si::Length> const& position)
{
	return tait_bryan_angles (ecef_orientation, polar (position));
}


inline SpaceMatrix<>
ecef_orientation (TaitBryanAngles const& angles, si::LonLat const& position)
{
	NorthEastDown const ned0 = ned_matrix (position);
	NorthEastDown const ned1 { rotation_about (ned0.down(), angles.yaw()) * static_cast<SpaceMatrix<>> (ned0) };
	NorthEastDown const ned2 { rotation_about (ned1.east(), angles.pitch()) * static_cast<SpaceMatrix<>> (ned1) };
	NorthEastDown const ned3 { rotation_about (ned2.north(), angles.roll()) * static_cast<SpaceMatrix<>> (ned2) };
	return ned3;
}


inline SpaceMatrix<>
ecef_orientation (TaitBryanAngles const& angles, SpaceVector<si::Length> const& position)
{
	return ecef_orientation (angles, polar (position));
}

} // namespace xf

#endif

