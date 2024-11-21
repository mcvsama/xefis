/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__IMPULSE_SOLVER_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__IMPULSE_SOLVER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/acceleration_moments.h>
#include <xefis/support/nature/force_moments.h>
#include <xefis/support/nature/velocity_moments.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/simulation/rigid_body/constraint.h>
#include <xefis/support/simulation/rigid_body/frame_precalculation.h>
#include <xefis/support/simulation/rigid_body/system.h>

// Neutrino:
#include <neutrino/noncopyable.h>
#include <neutrino/sequence.h>

// Standard:
#include <algorithm>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>


namespace xf::rigid_body {

/**
 * Physical quantities limits applied during system evolution.
 */
class Limits
{
  public:
	si::Force			max_force				{ 1e3_N };
	si::Torque			max_torque				{ 1e3_Nm };
	si::Velocity		max_velocity			{ 1e3_mps };
	si::AngularVelocity	max_angular_velocity	{ 1e3_radps };
};


/**
 * Used by ImpulseSolver to give information about each evolution details.
 */
class EvolutionDetails
{
  public:
	size_t	iterations_run	{ 0 };
	bool	converged		{ false };
};


/**
 * Simple impulse solver for rigid_body::System.
 */
class ImpulseSolver: private Noncopyable
{
	static constexpr size_t kDefaultMaxIterations { 1000 };

	struct ForceTorque
	{
		si::Force	force;
		si::Torque	torque;
	};

  public:
	/**
	 */
	explicit
	ImpulseSolver (System&, uint32_t max_iterations = kDefaultMaxIterations);

	/**
	 * Set limits applied during evolution.
	 */
	void
	set_limits (std::optional<Limits> const& limits)
		{ _limits = limits; }

	/**
	 * Set number of iterations of the converging algorithm.
	 */
	void
	set_max_iterations (size_t const max_iterations) noexcept
		{ _max_iterations = max_iterations; }

	/**
	 * Set required precision for calculated force moments of constraints.
	 */
	void
	set_required_precision (si::Force, si::Torque);

	/**
	 * With warm starting the constraint forces are reused for the next simulation frame
	 * as a starting points. It speeds up convergence.
	 * Enabled by default.
	 */
	void
	set_warm_starting (bool enabled)
		{ _warm_starting = enabled; }

	/**
	 * Evolve the system physically by given Δt.
	 */
	EvolutionDetails
	evolve (si::Time dt);

  private:
	void
	update_mass_moments();

	void
	update_gravitational_forces();

	static void
	update_gravitational_forces (Body&, Body&);

	void
	update_external_forces();

	void
	calculate_constants_for_step (si::Time dt);

	EvolutionDetails
	update_constraint_forces (si::Time dt);

	/**
	 * Return true if this constraint is solved withing required precision.
	 */
	[[nodiscard]]
	bool
	update_single_constraint_forces (Constraint*, si::Time dt);

	[[nodiscard]]
	static AccelerationMoments<WorldSpace>
	calculate_acceleration_moments (Placement<WorldSpace, BodyCOM> const&, MassMomentsAtCOM<BodyCOM> const&, ForceMoments<WorldSpace> const&);

	void
	update_acceleration_moments();

	[[nodiscard]]
	static VelocityMoments<WorldSpace>
	calculate_velocity_moments (VelocityMoments<WorldSpace>, AccelerationMoments<WorldSpace> const&, si::Time dt);

	void
	update_velocity_moments();

	[[nodiscard]]
	static Placement<WorldSpace, BodyCOM>
	calculate_placement (Placement<WorldSpace, BodyCOM>, VelocityMoments<WorldSpace> const&, si::Time const dt);

	void
	update_placements (si::Time dt);

	void
	normalize_rotations();

	template<class Space>
		void
		apply_limits (ForceMoments<Space>&) const;

	template<class Space>
		void
		apply_limits (VelocityMoments<Space>&) const;

  private:
	System&						_system;
	std::optional<Limits>		_limits;
	size_t						_max_iterations		{ kDefaultMaxIterations };
	uint64_t					_processed_frames	{ 0 };
	std::optional<ForceTorque>	_required_force_torque_precision;
	bool						_warm_starting		{ true };
};


template<class Space>
	inline void
	ImpulseSolver::apply_limits (ForceMoments<Space>& force_moments) const
	{
		if (_limits)
		{
			force_moments.set_force (length_limited (force_moments.force(), _limits->max_force));
			force_moments.set_torque (length_limited (force_moments.torque(), _limits->max_torque));
		}
	}


template<class Space>
	inline void
	ImpulseSolver::apply_limits (VelocityMoments<Space>& velocity_moments) const
	{
		if (_limits)
		{
			velocity_moments.set_velocity (length_limited (velocity_moments.velocity(), _limits->max_velocity));
			velocity_moments.set_angular_velocity (length_limited (velocity_moments.angular_velocity(), _limits->max_angular_velocity));
		}
	}

} // namespace xf::rigid_body

#endif

