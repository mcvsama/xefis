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

#ifndef XEFIS__SUPPORT__NATURE__VARIOUS_INERTIA_MATRICES_H__INCLUDED
#define XEFIS__SUPPORT__NATURE__VARIOUS_INERTIA_MATRICES_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


namespace xf {

template<class Space = void>
	inline SpaceMatrix<si::MomentOfInertia, Space>
	make_cube_inertia_matrix (si::Mass const& mass, SpaceVector<si::Length> const& dimensions)
	{
		auto const i00 = mass * (1.0 / 12.0) * (square (dimensions[1]) + square (dimensions[2]));
		auto const i11 = mass * (1.0 / 12.0) * (square (dimensions[0]) + square (dimensions[2]));
		auto const i22 = mass * (1.0 / 12.0) * (square (dimensions[0]) + square (dimensions[1]));
		auto const zero = 1_kg * 1_m2;

		return {
			i00, zero, zero,
			zero, i11, zero,
			zero, zero, i22,
		};
	}


template<class Space = void>
	inline SpaceMatrix<si::MomentOfInertia, Space>
	make_hollow_sphere_inertia_matrix (si::Mass const& mass, si::Length const radius)
	{
		auto const i = mass * (2.0 / 3.0) * square (radius);
		auto const zero = 1_kg * 1_m2;

		return {
			i, zero, zero,
			zero, i, zero,
			zero, zero, i,
		};
	}


template<class Space = void>
	inline SpaceMatrix<si::MomentOfInertia, Space>
	make_solid_sphere_inertia_matrix (si::Mass const& mass, si::Length const radius)
	{
		auto const i = mass * (2.0 / 5.0) * square (radius);
		auto const zero = 1_kg * 1_m2;

		return {
			i, zero, zero,
			zero, i, zero,
			zero, zero, i,
		};
	}


/**
 * The cylinder is considered to have length in the Z direction.
 */
template<class Space = void>
	inline SpaceMatrix<si::MomentOfInertia, Space>
	make_solid_cylinder_inertia_matrix (si::Mass const& mass, si::Length const radius, si::Length const length)
	{
		auto const i00 = mass * (1.0 / 12.0) * (3.0 * square (radius) + square (length));
		auto const i11 = mass * (1.0 / 12.0) * (3.0 * square (radius) + square (length));
		auto const i22 = mass * (1.0 / 2.0) * square (radius);
		auto const zero = 1_kg * 1_m2;

		return {
			i00, zero, zero,
			zero, i11, zero,
			zero, zero, i22,
		};
	}

} // namespace xf

#endif

