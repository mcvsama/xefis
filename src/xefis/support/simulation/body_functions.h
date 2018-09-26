/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__BODY_FUNCTIONS_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__BODY_FUNCTIONS_H__INCLUDED

// Standard:
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

// Lib:
#include <lib/math/math.h>
#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/sliced.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/c++20.h>
#include <xefis/support/math/3d_space.h>
#include <xefis/support/nature.h>
#include <xefis/support/simulation/body.h>


namespace xf::sim {

template<class Iterator, class ForceVectorIterator>
	void
	n_body_problem_forces (Iterator bodies_begin, Iterator bodies_end, ForceVectorIterator forces_begin, ForceVectorIterator forces_end)
	{
		static_assert (std::is_same_v<std::remove_cvref_t<decltype (*bodies_begin)>, sim::Body>, "body sequence must be iterators to xf::sim::Body");
		static_assert (std::is_same_v<std::remove_cvref_t<decltype (*forces_begin)>, SpaceVector<si::Force>>, "force seaquence must be iterators to xf::SpaceVector<si::Force>");

		auto bodies = boost::make_iterator_range (bodies_begin, bodies_end);
		auto forces = boost::make_iterator_range (forces_begin, forces_end);

		if (bodies.size() != forces.size())
			throw std::out_of_range ("body and force sequences have different sizes");

		std::fill (forces.begin(), forces.end(), xf::SpaceVector<si::Force> { 0_N, 0_N, 0_N });

		for (auto const i1: bodies | boost::adaptors::indexed())
		{
			for (auto const i2: bodies | boost::adaptors::indexed() | boost::adaptors::sliced (i1.index() + 1, bodies.size()))
			{
				auto const& b1 = i1.value();
				auto const& b2 = i2.value();

				auto const r = xf::abs (b2.position() - b1.position());
				auto const force = xf::kGravitationalConstant * b1.mass() * b2.mass() * (b2.position() - b1.position()) / (r * r * r);

				forces[i1.index()] += force;
				forces[i2.index()] -= force;
			}
		}
	}


template<class Iterator>
	std::vector<xf::SpaceVector<si::Force>>
	n_body_problem_forces (Iterator bodies_begin, Iterator bodies_end)
	{
		std::vector<xf::SpaceVector<si::Force>> forces (std::distance (bodies_begin, bodies_end));
		n_body_problem_forces (bodies_begin, bodies_end, forces.begin(), forces.end());
		return forces;
	}

} // namespace xf::sim

#endif

