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

#ifndef XEFIS__SUPPORT__NATURE__VARIOUS_INERTIA_TENSORS_H__INCLUDED
#define XEFIS__SUPPORT__NATURE__VARIOUS_INERTIA_TENSORS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


namespace xf {

struct MassRadius {
	si::Mass	mass;
	si::Length	radius;
};


struct MassRadiusLength {
	si::Mass	mass;
	si::Length	radius;
	si::Length	length;
};


/**
 * Center-of-mass is at the center of the cuboid.
 */
template<math::CoordinateSystem Space = void>
	inline InertiaTensor<Space>
	make_cuboid_inertia_tensor (si::Mass const& mass, SpaceLength<> const& dimensions)
	{
		auto const k = mass / 12.0;
		auto const i00 = k * (square (dimensions[1]) + square (dimensions[2]));
		auto const i11 = k * (square (dimensions[0]) + square (dimensions[2]));
		auto const i22 = k * (square (dimensions[0]) + square (dimensions[1]));
		auto const zero = 0_kg * 0_m2;

		return {
			i00, zero, zero,
			zero, i11, zero,
			zero, zero, i22,
		};
	}


template<math::CoordinateSystem Space = void>
	inline InertiaTensor<Space>
	make_cuboid_inertia_tensor (si::Mass const& mass, si::Length const edge_length)
	{
		return make_cuboid_inertia_tensor<Space> (mass, SpaceLength<> { edge_length, edge_length, edge_length });
	}


template<math::CoordinateSystem Space = void>
	inline InertiaTensor<Space>
	make_hollow_sphere_inertia_tensor (MassRadius const& params)
	{
		auto const i = params.mass * (2.0 / 3.0) * square (params.radius);
		auto const zero = 0_kg * 0_m2;

		return {
			i, zero, zero,
			zero, i, zero,
			zero, zero, i,
		};
	}


template<math::CoordinateSystem Space = void>
	inline InertiaTensor<Space>
	make_solid_sphere_inertia_tensor (MassRadius const& params)
	{
		auto const i = params.mass * (2.0 / 5.0) * square (params.radius);
		auto const zero = 0_kg * 0_m2;

		return {
			i, zero, zero,
			zero, i, zero,
			zero, zero, i,
		};
	}


/**
 * The cylinder is considered to have length in the Z direction.
 * The center of mass is at [0, 0, 0] position.
 */
template<math::CoordinateSystem Space = void>
	inline InertiaTensor<Space>
	make_centered_solid_cylinder_inertia_tensor (MassRadiusLength const& params)
	{
		auto const i00 = params.mass * (1.0 / 12.0) * (3.0 * square (params.radius) + square (params.length));
		auto const i11 = params.mass * (1.0 / 12.0) * (3.0 * square (params.radius) + square (params.length));
		auto const i22 = params.mass * (1.0 / 2.0) * square (params.radius);
		auto const zero = 0_kg * 0_m2;

		return {
			i00, zero, zero,
			zero, i11, zero,
			zero, zero, i22,
		};
	}

} // namespace xf

#endif

