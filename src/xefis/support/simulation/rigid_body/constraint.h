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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__CONSTRAINT_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__CONSTRAINT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/force_moments.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/simulation/rigid_body/connected_bodies.h>

// Neutrino:
#include <neutrino/logger.h>

// Standard:
#include <cstddef>
#include <optional>


namespace xf::rigid_body {

using ConstraintForces = std::array<ForceMoments<WorldSpace>, 2>;


/**
 * Constraints implementation is based on papers:
 * • "Constraints Derivation for Rigid Body Simulation in 3D", 13-11-2013 by Daniel Chappuis
 *   <www.danielchappuis.ch>
 *   (there were some small mistakes in some formulas, though).
 * • "Rigid Body Dynamics: Links and Joints", 16-09-2009 by Kristina Pickl
 */
class Constraint: public ConnectedBodies
{
  public:
	static constexpr double kDefaultBaumgarteFactor	= 0.5;

  public:
	// Dtor
	virtual
	~Constraint() = default;

	// Jacobian matrix for linear velocities:
	template<std::size_t N>
		using JacobianV = math::Matrix<double, 3, N, WorldSpace, WorldSpace>;

	// Jacobian matrix for angular velocities:
	template<std::size_t N>
		using JacobianW = math::Matrix<si::Length, 3, N, WorldSpace, WorldSpace>;

	// Total integrated jacobian:
	template<std::size_t N>
		using Jacobian = math::Vector<si::Velocity, N, WorldSpace>;

	// Location constraint vector (angles are represented by axis-angle vector):
	template<std::size_t N>
		using LocationConstraint = math::Vector<si::Length, N, WorldSpace>;

	// Lambda:
	template<std::size_t N>
		using Lambda = math::Vector<si::Force, N, WorldSpace>;

	// Constraint mass matrix:
	template<std::size_t N>
		using ConstraintMassMatrix = math::SquareMatrix<decltype (1 / 1_kg), N, WorldSpace, WorldSpace>;

	// Z matrix is a result of -inv (K) * dt, where K is result of compute_K():
	template<std::size_t N>
		using ConstraintZMatrix = math::SquareMatrix<decltype (1_kg / 1_s), N, WorldSpace, WorldSpace>;

  public:
	// Ctor
	using ConnectedBodies::ConnectedBodies;

	// Ctor
	explicit
	Constraint (ConnectedBodies const&);

	[[nodiscard]]
	std::string const&
	label() const noexcept
		{ return _label; }

	void
	set_label (std::string const& label)
		{ _label = label; }

	/**
	 * Return true if constraint is enabled.
	 * Constraint is enabled by default.
	 */
	[[nodiscard]]
	bool
	enabled() const noexcept
		{ return _enabled; }

	/**
	 * Enable/disable constraint.
	 */
	void
	set_enabled (bool enabled)
		{ _enabled = enabled; }

	/**
	 * Return breaking force.
	 */
	[[nodiscard]]
	std::optional<si::Force>
	breaking_force() const noexcept
		{ return _breaking_force; }

	/**
	 * Set breaking force, that is if constraint force is greater than configured, constraint will become "broken".
	 */
	void
	set_breaking_force (std::optional<si::Force> const breaking_force)
		{ _breaking_force = breaking_force; }

	/**
	 * Return breaking torque.
	 */
	[[nodiscard]]
	std::optional<si::Torque>
	breaking_torque() const noexcept
		{ return _breaking_torque; }

	/**
	 * Set breaking torque, that is if constraint torque is greater than configured, constraint will become "broken".
	 */
	void
	set_breaking_torque (std::optional<si::Torque> const breaking_torque)
		{ _breaking_torque = breaking_torque; }

	/**
	 * Set breaking force/torque.
	 */
	void
	set_breaking_force_torque (std::optional<si::Force> const breaking_force, std::optional<si::Torque> const breaking_torque);

	/**
	 * Return true if constraint has become broken.
	 */
	[[nodiscard]]
	bool
	broken() const noexcept
		{ return _broken; }

	/**
	 * Break the constraint.
	 */
	void
	set_broken() noexcept
		{ _broken = true; }

	/**
	 * Baumgarte stabilization factor.
	 */
	[[nodiscard]]
	double
	baumgarte_factor() const noexcept
		{ return _baumgarte_factor; }

	/**
	 * Set Baumgarte stabilization factor.
	 */
	void
	set_baumgarte_factor (double factor) noexcept
		{ _baumgarte_factor = factor; }

	/**
	 * Constraint Force Mixing (CFM) factor.
	 * It effectively introduces a tiny compliance in constraints, causing them
	 * to dissipate energy through minor deviations rather than perfectly rigid enforcement.
	 * Makes simulation a bit more stable numerically and also can be used to remove
	 * bits of energy from the system.
	 */
	[[nodiscard]]
	double
	constraint_force_mixing_factor() const noexcept
		{ return _constraint_force_mixing_factor.value(); }

	/**
	 * Set Constraint Force Mixing factor.
	 * By default it's set to 0.
	 */
	void
	set_constraint_force_mixing_factor (double factor) noexcept
		{ _constraint_force_mixing_factor = ConstraintMassMatrix<0>::Scalar (factor); }

	/**
	 * Return the friction factor.
	 */
	[[nodiscard]]
	double
	friction_factor() const noexcept
		{ return _friction_factor; }

	/**
	 * Apply friction factor to simulate energy dissipation on the constraint.
	 * The factor should be small, usually between 0.001 and 0.01 for realism.
	 * By default it's 0.
	 */
	void
	set_friction_factor (double factor) noexcept
		{ _friction_factor = factor; }

	/**
	 * Initialize the constraint for the next simulation step (frame).
	 * It's not called between solver iterations, only at each new step.
	 * Default implementation does nothing.
	 */
	virtual void
	initialize_step ([[maybe_unused]] si::Time dt)
	{ }

	/**
	 * Return constraint forces to apply to the two bodies.
	 *
	 * Calls do_constraint_forces() and check if constraint needs to be broken
	 * based on set breaking force/torque.
	 */
	[[nodiscard]]
	ConstraintForces
	constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt);

	/**
	 * Called when final constraint forces are obtained for current frame of simulation.
	 */
	virtual void
	computed_constraint_forces (ConstraintForces const& result, [[maybe_unused]] si::Time const dt)
	{
		if (_breaking_force)
			if (abs (result[0].force()) > *_breaking_force || abs (result[1].force()) > *_breaking_force)
				_broken = true;

		if (_breaking_torque)
			if (abs (result[0].torque()) > *_breaking_torque || abs (result[1].torque()) > *_breaking_torque)
				_broken = true;
	}

	/**
	 * Access the previous computation ForceMoments used by the solver
	 * to figure out if required simulation precision has been acquired
	 * for this constraint.
	 */
	std::optional<ForceMoments<WorldSpace>>&
	previous_computation_force_moments()
		{ return _previous_computation_force_moments; }

  protected:
	/**
	 * Should return computed constraint forces.
	 */
	[[nodiscard]]
	virtual ConstraintForces
	do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt) = 0;

	/**
	 * Helper function that computes actual corrective forces for given Jacobians and location constraints.
	 *
	 * \param	ext_forces_1, ext_forces_2
	 *			External force-moments acting on bodies 1, 2.
	 */
	template<std::size_t N>
		[[nodiscard]]
		ConstraintForces
		compute_constraint_forces (JacobianV<N> const& Jv1,
								   JacobianW<N> const& Jw1,
								   JacobianV<N> const& Jv2,
								   JacobianW<N> const& Jw2,
								   Lambda<N> const& lambda) const;

	/**
	 * Calculates total jacobian (J[i]) for current simulation frame.
	 */
	template<std::size_t N>
		[[nodiscard]]
		Jacobian<N>
		compute_jacobian (VelocityMoments<WorldSpace> const& vm_1,
						  JacobianV<N> const& Jv1,
						  JacobianW<N> const& Jw1,
						  VelocityMoments<WorldSpace> const& vm_2,
						  JacobianV<N> const& Jv2,
						  JacobianW<N> const& Jw2) const;

	/**
	 * Calculate lambda (a vector of si::Force).
	 */
	template<std::size_t N>
		[[nodiscard]]
		Lambda<N>
		compute_lambda (LocationConstraint<N> const& location_constraint,
						Jacobian<N> const& J,
						ConstraintMassMatrix<N> const& K,
						si::Time dt) const;

	/**
	 * Calculate lambda (a vector of si::Force).
	 */
	template<std::size_t N>
		[[nodiscard]]
		Lambda<N>
		compute_lambda (LocationConstraint<N> const& location_constraint,
						Jacobian<N> const& J,
						ConstraintZMatrix<N> const& Z,
						si::Time dt) const;

	/**
	 * Calculate mass matrix K in a generic way.
	 * It's also called the "constraint matrix".
	 */
	template<std::size_t N>
		[[nodiscard]]
		ConstraintMassMatrix<N>
		compute_K (JacobianV<N> const& Jv1,
				   JacobianW<N> const& Jw1,
				   JacobianV<N> const& Jv2,
				   JacobianW<N> const& Jw2) const;

	/**
	 * Calculate mass matrix K assuming that angular-velocity Jacobians are 0⃗.
	 */
	template<std::size_t N>
		[[nodiscard]]
		ConstraintMassMatrix<N>
		compute_K (JacobianV<N> const& Jv1,
				   JacobianV<N> const& Jv2) const;

	/**
	 * Calculate mass matrix K assuming that linear-velocity Jacobians are 0⃗.
	 */
	template<std::size_t N>
		[[nodiscard]]
		ConstraintMassMatrix<N>
		compute_K (JacobianW<N> const& Jw1,
				   JacobianW<N> const& Jw2) const;

	template<std::size_t N>
		[[nodiscard]]
		ConstraintZMatrix<N>
		compute_Z (JacobianV<N> const& Jv1,
				   JacobianW<N> const& Jw1,
				   JacobianV<N> const& Jv2,
				   JacobianW<N> const& Jw2,
				   si::Time const dt) const
		{ return -1.0 / dt * inv (compute_K (Jv1, Jw1, Jv2, Jw2)); }

	template<std::size_t N>
		[[nodiscard]]
		ConstraintZMatrix<N>
		compute_Z (JacobianV<N> const& Jv1,
				   JacobianV<N> const& Jv2,
				   si::Time const dt) const
		{ return -1.0 / dt * inv (compute_K (Jv1, Jv2)); }

	template<std::size_t N>
		[[nodiscard]]
		ConstraintZMatrix<N>
		compute_Z (JacobianW<N> const& Jw1,
				   JacobianW<N> const& Jw2,
				   si::Time const dt) const
		{ return -1.0 / dt * inv (compute_K (Jw1, Jw2)); }

	/**
	 * Adds Constraint Mixing Factor (CFM) to the given K matrix.
	 */
	template<std::size_t N>
		ConstraintMassMatrix<N>&
		apply_constraint_mixing_factor (ConstraintMassMatrix<N>&) const;

  private:
	std::string						_label;
	bool							_enabled						{ true };
	bool							_broken							{ false };
	std::optional<si::Force>		_breaking_force;
	std::optional<si::Torque>		_breaking_torque;
	double							_baumgarte_factor				{ kDefaultBaumgarteFactor };
	ConstraintMassMatrix<0>::Scalar	_constraint_force_mixing_factor { 0.0 };
	double							_friction_factor				{ 0.0 }; // TODO implement
	std::optional<ForceMoments<WorldSpace>>
									_previous_computation_force_moments;
};


inline
Constraint::Constraint (ConnectedBodies const& connected_bodies):
	ConnectedBodies (connected_bodies)
{ }


inline void
Constraint::set_breaking_force_torque (std::optional<si::Force> const breaking_force, std::optional<si::Torque> const breaking_torque)
{
	set_breaking_force (breaking_force);
	set_breaking_torque (breaking_torque);
}


inline ConstraintForces
Constraint::constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt)
{
	if (_broken)
	{
		return {
			ForceMoments<WorldSpace> { math::zero, math::zero },
			ForceMoments<WorldSpace> { math::zero, math::zero },
		};
	}
	else
		return do_constraint_forces (vm_1, vm_2, dt);

}


template<std::size_t N>
	inline ConstraintForces
	Constraint::compute_constraint_forces (JacobianV<N> const& Jv1,
										   JacobianW<N> const& Jw1,
										   JacobianV<N> const& Jv2,
										   JacobianW<N> const& Jw2,
										   Lambda<N> const& lambda) const
	{
		auto const Fc1 = ~Jv1 * lambda;
		auto const Tc1 = ~Jw1 * lambda;
		auto const Fc2 = ~Jv2 * lambda;
		auto const Tc2 = ~Jw2 * lambda;

		return {
			ForceMoments<WorldSpace> (Fc1, Tc1),
			ForceMoments<WorldSpace> (Fc2, Tc2),
		};
	}


template<std::size_t N>
	inline Constraint::Jacobian<N>
	Constraint::compute_jacobian (VelocityMoments<WorldSpace> const& vm_1,
								  JacobianV<N> const& Jv1,
								  JacobianW<N> const& Jw1,
								  VelocityMoments<WorldSpace> const& vm_2,
								  JacobianV<N> const& Jv2,
								  JacobianW<N> const& Jw2) const
	{
		constexpr auto inv_radian = decltype (1.0 / 1_rad) { 1 };

		auto const& b1_iter = body_1().iteration();
		auto const& b2_iter = body_2().iteration();

		// Total jacobian: J * (v + Δt * a)
		auto const J = Jv1 * (vm_1.velocity() + b1_iter.external_impulses_over_mass)
					 + Jw1 * (vm_1.angular_velocity() * inv_radian + b1_iter.external_angular_impulses_over_inertia_tensor)
					 + Jv2 * (vm_2.velocity() + b2_iter.external_impulses_over_mass)
					 + Jw2 * (vm_2.angular_velocity() * inv_radian + b2_iter.external_angular_impulses_over_inertia_tensor);

		return J;
	}


template<std::size_t N>
	inline Constraint::Lambda<N>
	Constraint::compute_lambda (LocationConstraint<N> const& location_constraint,
								Jacobian<N> const& J,
								ConstraintMassMatrix<N> const& K,
								si::Time const dt) const
	{
		auto const inv_dt = 1 / dt;
		auto const stabilization_bias = baumgarte_factor() * inv_dt * location_constraint;
		return -inv_dt * inv (K) * (J + stabilization_bias);
	}


template<std::size_t N>
	inline Constraint::Lambda<N>
	Constraint::compute_lambda (LocationConstraint<N> const& location_constraint,
								Jacobian<N> const& J,
								ConstraintZMatrix<N> const& Z,
								si::Time const dt) const
	{
		auto const inv_dt = 1 / dt;
		auto const stabilization_bias = baumgarte_factor() * inv_dt * location_constraint;
		return Z * (J + stabilization_bias);
	}


template<std::size_t N>
	inline Constraint::ConstraintMassMatrix<N>
	Constraint::compute_K (JacobianV<N> const& Jv1,
						   JacobianW<N> const& Jw1,
						   JacobianV<N> const& Jv2,
						   JacobianW<N> const& Jw2) const
	{
		auto const& inv_M1 = body_1().iteration().inv_M;
		auto const& inv_I1 = body_1().iteration().inv_I;
		auto const& inv_M2 = body_2().iteration().inv_M;
		auto const& inv_I2 = body_2().iteration().inv_I;

		// Unfolded expression: J * inv(M) * ~J.
		// This has to be unfolded because of two distinct scalar types held by Jacobians.
		auto K = Jv1 * inv_M1 * ~Jv1
			   + Jw1 * inv_I1 * ~Jw1
			   + Jv2 * inv_M2 * ~Jv2
			   + Jw2 * inv_I2 * ~Jw2;

		return apply_constraint_mixing_factor (K);
	}


template<std::size_t N>
	inline Constraint::ConstraintMassMatrix<N>
	Constraint::compute_K (JacobianV<N> const& Jv1,
						   JacobianV<N> const& Jv2) const
	{
		auto const& inv_M1 = body_1().iteration().inv_M;
		auto const& inv_M2 = body_2().iteration().inv_M;

		// Unfolded expression: J * inv(M) * ~J.
		auto K = Jv1 * inv_M1 * ~Jv1
			   + Jv2 * inv_M2 * ~Jv2;

		return apply_constraint_mixing_factor (K);
	}


template<std::size_t N>
	inline Constraint::ConstraintMassMatrix<N>
	Constraint::compute_K (JacobianW<N> const& Jw1,
						   JacobianW<N> const& Jw2) const
	{
		auto const& inv_I1 = body_1().iteration().inv_I;
		auto const& inv_I2 = body_2().iteration().inv_I;

		// Unfolded expression: J * inv(M) * ~J.
		auto K = Jw1 * inv_I1 * ~Jw1
			   + Jw2 * inv_I2 * ~Jw2;

		return apply_constraint_mixing_factor (K);
	}


template<std::size_t N>
	inline Constraint::ConstraintMassMatrix<N>&
	Constraint::apply_constraint_mixing_factor (ConstraintMassMatrix<N>& K) const
	{
		if (_constraint_force_mixing_factor.value() != 0.0)
			for (std::size_t i = 0; i < N; ++i)
				K[i, i] += _constraint_force_mixing_factor;

		return K;
	}


/*
 * Global functions
 */


inline ConstraintForces&
operator+= (ConstraintForces& a, ConstraintForces const& b)
{
	for (std::size_t i = 0; i < 2; ++i)
		a[i] -= b[i];

	return a;
}


inline ConstraintForces&
operator-= (ConstraintForces& a, ConstraintForces const& b)
{
	for (std::size_t i = 0; i < 2; ++i)
		a[i] -= b[i];

	return a;
}


inline ConstraintForces
operator+ (ConstraintForces a, ConstraintForces const& b)
{
	for (std::size_t i = 0; i < 2; ++i)
		a[i] += b[i];

	return a;
}


inline ConstraintForces
operator- (ConstraintForces a, ConstraintForces const& b)
{
	for (std::size_t i = 0; i < 2; ++i)
		a[i] -= b[i];

	return a;
}

} // namespace xf::rigid_body

#endif

