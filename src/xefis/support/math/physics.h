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

#ifndef XEFIS__SUPPORT__MATH__PHYSICS_H__INCLUDED
#define XEFIS__SUPPORT__MATH__PHYSICS_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <boost/range.hpp>
#include <lib/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/3d_space.h>


namespace xf {

template<class PointMassIterator>
	xf::SpaceVector<si::Length>
	center_of_gravity (PointMassIterator masses_begin, PointMassIterator masses_end)
	{
		xf::SpaceVector<decltype (si::Length{} * si::Mass{})> center (math::ZeroMatrix);
		si::Mass total_mass = 0_kg;

		for (auto const& mx: boost::make_iterator_range (masses_begin, masses_end))
		{
			auto const x = std::get<xf::SpaceVector<si::Length>> (mx);
			auto const m = std::get<si::Mass> (mx);

			center += m * x;
			total_mass += m;
		}

		return 1.0 / total_mass * center;
	}


template<class PointMassIterator>
	xf::SpaceMatrix<si::MomentOfInertia>
	moment_of_inertia (PointMassIterator masses_begin, PointMassIterator masses_end)
	{
		xf::SpaceMatrix<si::MomentOfInertia> sum (math::ZeroMatrix);

		for (auto const& mx: boost::make_iterator_range (masses_begin, masses_end))
		{
			auto const x = std::get<xf::SpaceVector<si::Length>> (mx);
			auto const m = std::get<si::Mass> (mx);

			sum += m * (-x * x.transposed());
		}

		return sum;
	}


/**
 * Move mass distributions so that position [0, 0, 0] is in center of gravity.
 */
template<class PointMassIterator>
	inline void
	move_to_center_of_gravity (PointMassIterator begin, PointMassIterator end)
	{
		auto const cog = xf::center_of_gravity (begin, end);

		for (auto& point_mass: boost::make_iterator_range (begin, end))
			std::get<xf::SpaceVector<si::Length>> (point_mass) -= cog;
	}


/**
 * Return sum of point masses.
 */
template<class PointMassIterator>
	inline si::Mass
	total_mass (PointMassIterator begin, PointMassIterator end)
	{
		auto sum = 0_kg;

		for (auto& point_mass: boost::make_iterator_range (begin, end))
			sum += std::get<si::Mass> (point_mass);

		return sum;
	}

} // namespace xf

#endif

