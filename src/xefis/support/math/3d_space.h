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

// Local:
#include "lonlat_radius.h"


namespace xf {

template<class Value>
	using SpaceVector = math::Vector<Value, 3>;

template<class Value>
	using SpaceMatrix = math::Matrix<Value, 3, 3>;

using SpaceQuaternion = math::Quaternion<double>;


/**
 * Return rotation matrix along the axis X for given angle.
 */
inline math::Matrix<double, 3, 3>
rotation_x (si::Angle const& angle)
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
inline math::Matrix<double, 3, 3>
rotation_y (si::Angle const& angle)
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
inline math::Matrix<double, 3, 3>
rotation_z (si::Angle const& angle)
{
	double const sin_a = sin (angle);
	double const cos_a = cos (angle);

	return {
		+cos_a, -sin_a, 0.0,
		+sin_a, +cos_a, 0.0,
		   0.0,    0.0, 1.0,
	};
}


inline SpaceVector<si::Length>
to_cartesian (LonLatRadius const& position)
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
to_polar (SpaceVector<si::Length> const& vector)
{
	std::complex<double> const xy (vector[0].quantity(), vector[1].quantity());
	std::complex<double> const wz (std::abs (xy), vector[2].quantity());

	return LonLatRadius {
		si::LonLat (1_rad * std::arg (xy), 1_rad * std::arg (wz)),
		si::Length (std::abs (wz))
	};
}


/**
 * Return length of given vector.
 */
template<class Value>
	inline Value
	abs (math::Vector<Value, 3> const& vector)
	{
		using std::sqrt;

		return sqrt (vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
	}

} // namespace xf

#endif

