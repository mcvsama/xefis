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

#ifndef XEFIS__SUPPORT__SIMULATION__N_BODY__BODY_FUNCTIONS_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__N_BODY__BODY_FUNCTIONS_H__INCLUDED

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
#include <xefis/support/math/lonlat_radius.h>
#include <xefis/support/math/space.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/simulation/n_body/body.h>
#include <xefis/support/simulation/n_body/body_part.h>


namespace xf::sim {

[[nodiscard]]
inline Body
make_earth()
{
	// Simplified EGM96 model:
	SpaceMatrix<si::MomentOfInertia, PartFrame> const earth_moment_of_inertia {
		8.008085e37_kgm2,           0_kgm2,           0_kgm2,
		          0_kgm2, 8.008262e37_kgm2,           0_kgm2,
		          0_kgm2,           0_kgm2, 8.034476e37_kgm2,
	};

	BodyShape shape;
	shape.add (std::make_unique<BodyPart<AirframeFrame, PartFrame>> (PositionRotation<AirframeFrame, PartFrame>(),
																	 kEarthMass,
																	 earth_moment_of_inertia));

	Body earth (std::move (shape));
	earth.set_position ({ 0_m, 0_m, 0_m });
	earth.set_velocity ({ 0_mps, 0_mps, 0_mps });
	// Since we use ECEF coordinates, don't spin the Earth:
	earth.set_angular_velocity ({ si::convert (0_radps), si::convert (0_radps), si::convert (0_radps) });

	return earth;
}


/**
 * Note: does not compute torques resulting from different gravitational pulls on different places on the body.
 */
template<class Frame, class Iterator, class ForceTorqueIterator>
	inline void
	n_body_problem_forces (Iterator bodies_begin, Iterator bodies_end, ForceTorqueIterator forces_begin, ForceTorqueIterator forces_end)
	{
		static_assert (std::is_same_v<std::remove_cvref_t<decltype (*bodies_begin)>, Body*>, "body sequence must be iterators to xf::sim::Body*");
		static_assert (std::is_same_v<std::remove_cvref_t<decltype (*forces_begin)>, ForceTorque<Frame>>, "force seaquence must be iterators to xf::ForceTorque");

		auto bodies = boost::make_iterator_range (bodies_begin, bodies_end);
		auto forces = boost::make_iterator_range (forces_begin, forces_end);

		if (bodies.size() != forces.size())
			throw std::out_of_range ("body and force sequences have different sizes");

		std::fill (forces.begin(), forces.end(), ForceTorque<Frame>());

		for (auto const i1: bodies | boost::adaptors::indexed())
		{
			for (auto const i2: bodies | boost::adaptors::indexed() | boost::adaptors::sliced (i1.index() + 1, bodies.size()))
			{
				auto const& b1 = *i1.value();
				auto const& b2 = *i2.value();

				auto const r = abs (b2.position() - b1.position());
				auto const force = kGravitationalConstant * b1.shape().mass() * b2.shape().mass() * (b2.position() - b1.position()) / (r * r * r);

				auto& f1 = forces[i1.index()];
				auto& f2 = forces[i2.index()];
				f1.set_force (f1.force() + force);
				f2.set_force (f2.force() - force);
			}
		}
	}


template<class Frame, class Iterator>
	[[nodiscard]]
	inline std::vector<ForceTorque<Frame>>
	n_body_problem_forces (Iterator bodies_begin, Iterator bodies_end)
	{
		std::vector<ForceTorque<Frame>> forces (std::distance (bodies_begin, bodies_end));
		n_body_problem_forces<Frame> (bodies_begin, bodies_end, forces.begin(), forces.end());
		return forces;
	}

} // namespace xf::sim

#endif

