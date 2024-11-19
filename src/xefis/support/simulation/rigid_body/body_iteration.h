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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__BODY_ITERATION_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__BODY_ITERATION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/acceleration_moments.h>
#include <xefis/support/nature/force_moments.h>
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/nature/velocity_moments.h>
#include <xefis/support/simulation/rigid_body/concepts.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

/**
 * A set of calculations related to the body done by the simulator on each iteration
 * when converging.
 */
class BodyIteration
{
  public:
	// Those are recalculated on each simulation step, but stay the same on all
	// solver iterations:
	SpaceMatrix<si::Mass, WorldSpace>::InverseMatrix			inv_M;
	SpaceMatrix<si::MomentOfInertia, WorldSpace>::InverseMatrix	inv_I;

	// Those are used temporarily when calculating all_constraints_force_moments:
	ForceMoments<WorldSpace>									gravitational_force_moments;
	ForceMoments<WorldSpace>									external_force_moments; // Excluding gravitation.
	VelocityMoments<WorldSpace>									velocity_moments;

	// Needed by Body::acceleration_moments_except_gravity(): TODO maybe it can be moved to the Body?
	AccelerationMoments<WorldSpace>								acceleration_moments_except_gravity;

	// The resulting summed constraint forces to apply to the body after simulation step:
	ForceMoments<WorldSpace>									all_constraints_force_moments;

  public:
	[[nodiscard]]
	ForceMoments<WorldSpace>
	all_force_moments() const noexcept
		{ return gravitational_force_moments + external_force_moments + all_constraints_force_moments; }

	[[nodiscard]]
	ForceMoments<WorldSpace>
	force_moments_except_gravity() const noexcept
		{ return external_force_moments + all_constraints_force_moments; }
};

} // namespace xf::rigid_body

#endif

