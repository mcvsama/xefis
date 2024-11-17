/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__MATH__GEOMETRY_TYPES_H__INCLUDED
#define XEFIS__SUPPORT__MATH__GEOMETRY_TYPES_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/math/concepts.h>
#include <neutrino/math/math.h>

// Standard:
#include <cstddef>


namespace xf {

template<class Scalar = double, math::CoordinateSystem Space = void>
	using PlaneVector = math::Vector<Scalar, 2, Space, void>;

template<class Scalar = double, math::CoordinateSystem Space = void>
	using SpaceVector = math::Vector<Scalar, 3, Space, void>;

template<class Scalar = double, math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	using PlaneMatrix = math::Matrix<Scalar, 2, 2, TargetSpace, SourceSpace>;

template<class Scalar = double, math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	using SpaceMatrix = math::Matrix<Scalar, 3, 3, TargetSpace, SourceSpace>;

/*
 * Transforms
 */

// Essentially same as RotationMatrix but use RotationMatrix for intended rotations,
// and AffineTransform for everything.
template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	using AffineTransform = SpaceMatrix<double, TargetSpace, SourceSpace>;

template<class Scalar, std::size_t N, math::CoordinateSystem Space = void>
	using Triangle = std::array<math::Vector<Scalar, N, Space>, 3>;

template<class Scalar, math::CoordinateSystem Space = void>
	using PlaneTriangle = Triangle<Scalar, 2, Space>;

template<class Scalar, math::CoordinateSystem Space = void>
	using SpaceTriangle = Triangle<Scalar, 3, Space>;

template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	using RotationMatrix = SpaceMatrix<double, TargetSpace, SourceSpace>;

template<math::CoordinateSystem TargetSpace = void, math::CoordinateSystem SourceSpace = TargetSpace>
	using RotationQuaternion = math::Quaternion<double, TargetSpace, SourceSpace>;

/*
 * Physical stuff
 */

template<math::CoordinateSystem Space = void>
	using SpaceLength = SpaceVector<si::Length, Space>;

template<math::CoordinateSystem Space = void>
	using SpaceForce = SpaceVector<si::Force, Space>;

template<math::CoordinateSystem Space = void>
	using SpaceTorque = SpaceVector<si::Torque, Space>;

} // namespace xf

#endif

