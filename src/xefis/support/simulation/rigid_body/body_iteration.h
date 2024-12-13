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
	using ImpulseOverMass = decltype (si::Impulse (1) / si::Mass (1));
	using AngularImpulseOverInertia = decltype (si::AngularImpulse (1) / si::MomentOfInertia (1));

  public:
	// Those are recalculated on each simulation step, but stay the same on all
	// solver iterations:
	SpaceMatrix<si::Mass, WorldSpace>::InverseMatrix			inv_M;
	SpaceMatrix<si::MomentOfInertia, WorldSpace>::InverseMatrix	inv_I;
	ForceMoments<WorldSpace>									gravitational_force_moments;
	ForceMoments<WorldSpace>									external_force_moments_except_gravity; // Excluding gravitation.
	ForceMoments<WorldSpace>									external_force_moments; // Gravity + external_force_moments_except_gravity
	SpaceVector<ImpulseOverMass, WorldSpace>					external_impulses_over_mass;
	SpaceVector<AngularImpulseOverInertia, WorldSpace>			external_angular_impulses_over_inertia_tensor;

	// Those are used temporarily when calculating all_constraints_force_moments:
	VelocityMoments<WorldSpace>									velocity_moments;
	bool														velocity_moments_updated { false };
	std::optional<AccelerationMoments<WorldSpace>>				acceleration_moments;

	// The resulting summed constraint forces to apply to the body after simulation step:
	ForceMoments<WorldSpace>									all_constraints_force_moments;

  public:
	/**
	 * Reset values for new iteration. Only resets stuff that needs reset.
	 */
	void
	reset (VelocityMoments<WorldSpace> const& vm);

	[[nodiscard]]
	ForceMoments<WorldSpace>
	all_force_moments() const noexcept
		{ return external_force_moments + all_constraints_force_moments; }

	[[nodiscard]]
	ForceMoments<WorldSpace>
	force_moments_except_gravity() const noexcept
		{ return external_force_moments_except_gravity + all_constraints_force_moments; }
};


inline void
BodyIteration::reset (VelocityMoments<WorldSpace> const& vm)
{
	this->velocity_moments = vm; // TODO + warmer;
	this->velocity_moments_updated = false;
	this->acceleration_moments.reset();
}

} // namespace xf::rigid_body

#endif

