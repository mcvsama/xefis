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

// Standard:
#include <cstddef>
#include <optional>

// Neutrino:
#include <neutrino/logger.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/force_moments.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/simulation/rigid_body/connected_bodies.h>


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

	// Location constraint vector (angles are represented by axis-angle vector):
	template<std::size_t N>
		using LocationConstraint = math::Vector<si::Length, N, WorldSpace>;

	// Constraint mass matrix:
	template<std::size_t N>
		using ConstraintMassMatrix = math::SquareMatrix<decltype (1 / 1_kg), N, WorldSpace, WorldSpace>;

  public:
	// Ctor
	using ConnectedBodies::ConnectedBodies;

	// Ctor
	explicit
	Constraint (ConnectedBodies const&);

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
	 * Return constraint forces to apply to the two bodies.
	 *
	 * Call do_constraint_forces() and check if constraint needs to be broken
	 * based on set breaking force/torque.
	 */
	[[nodiscard]]
	ConstraintForces
	constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
					   VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
					   si::Time dt);

	/**
	 * Called when final constraint forces are obtained for current frame of simulation.
	 */
	virtual void
	calculated_constraint_forces (ConstraintForces const&)
	{ }

  protected:
	/**
	 * Should return calculated constraint forces.
	 */
	[[nodiscard]]
	virtual ConstraintForces
	do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
						  VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
						  si::Time dt) = 0;

	/**
	 * Helper function to calculate actual corrective forces for given Jacobians and location constraints.
	 *
	 * \param	ext_forces_1, ext_forces_2
	 *			External force-moments acting on bodies 1, 2.
	 */
	template<std::size_t N>
		[[nodiscard]]
		ConstraintForces
		calculate_constraint_forces (VelocityMoments<WorldSpace> const& vm_1,
									 ForceMoments<WorldSpace> const& ext_forces_1,
									 JacobianV<N> const& Jv1,
									 JacobianW<N> const& Jw1,
									 VelocityMoments<WorldSpace> const& vm_2,
									 ForceMoments<WorldSpace> const& ext_forces_2,
									 JacobianV<N> const& Jv2,
									 JacobianW<N> const& Jw2,
									 LocationConstraint<N> const&,
									 ConstraintMassMatrix<N> const&,
									 si::Time dt) const;

	/**
	 * Calculate mass matrix K in a generic way.
	 */
	template<std::size_t N>
		[[nodiscard]]
		ConstraintMassMatrix<N>
		calculate_K (JacobianV<N> const& Jv1,
					 JacobianW<N> const& Jw1,
					 JacobianV<N> const& Jv2,
					 JacobianW<N> const& Jw2) const;

	/**
	 * Calculate mass matrix K assuming that angular-velocity Jacobians are 0⃗.
	 */
	template<std::size_t N>
		[[nodiscard]]
		ConstraintMassMatrix<N>
		calculate_K (JacobianV<N> const& Jv1,
					 JacobianV<N> const& Jv2) const;

	/**
	 * Calculate mass matrix K assuming that linear-velocity Jacobians are 0⃗.
	 */
	template<std::size_t N>
		[[nodiscard]]
		ConstraintMassMatrix<N>
		calculate_K (JacobianW<N> const& Jw1,
					 JacobianW<N> const& Jw2) const;

  private:
	bool						_enabled			{ true };
	bool						_broken				{ false };
	std::optional<si::Force>	_breaking_force;
	std::optional<si::Torque>	_breaking_torque;
	double						_baumgarte_factor	{ kDefaultBaumgarteFactor };
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
Constraint::constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
							   VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
							   si::Time dt)
{
	auto result = do_constraint_forces (vm_1, ext_forces_1, vm_2, ext_forces_2, dt);

	if (_breaking_force)
		if (abs (result[0].force()) > *_breaking_force || abs (result[1].force()) > *_breaking_force)
			_broken = true;

	if (_breaking_torque)
		if (abs (result[0].torque()) > *_breaking_torque || abs (result[1].torque()) > *_breaking_torque)
			_broken = true;

	if (_broken)
	{
		return {
			ForceMoments<WorldSpace> { math::zero, math::zero },
			ForceMoments<WorldSpace> { math::zero, math::zero },
		};
	}
	else
		return result;
}


template<std::size_t N>
	inline ConstraintForces
	Constraint::calculate_constraint_forces (VelocityMoments<WorldSpace> const& vm_1,
											 ForceMoments<WorldSpace> const& ext_forces_1,
											 JacobianV<N> const& Jv1,
											 JacobianW<N> const& Jw1,
											 VelocityMoments<WorldSpace> const& vm_2,
											 ForceMoments<WorldSpace> const& ext_forces_2,
											 JacobianV<N> const& Jv2,
											 JacobianW<N> const& Jw2,
											 LocationConstraint<N> const& location_constraint,
											 ConstraintMassMatrix<N> const& K,
											 si::Time dt) const
	{
		auto const v1 = vm_1.velocity();
		auto const w1 = vm_1.angular_velocity() / 1_rad;
		auto const v2 = vm_2.velocity();
		auto const w2 = vm_2.angular_velocity() / 1_rad;

		auto const inv_M1 = body_1().frame_cache().inv_M;
		auto const inv_I1 = body_1().frame_cache().inv_I;
		auto const inv_M2 = body_2().frame_cache().inv_M;
		auto const inv_I2 = body_2().frame_cache().inv_I;

		// J * (v + Δt * a)
		auto const Jvi = Jv1 * (v1 + dt * inv_M1 * (ext_forces_1.force()))
					   + Jw1 * (w1 + dt * inv_I1 * (ext_forces_1.torque()))
					   + Jv2 * (v2 + dt * inv_M2 * (ext_forces_2.force()))
					   + Jw2 * (w2 + dt * inv_I2 * (ext_forces_2.torque()));

		auto const stabilization_bias = baumgarte_factor() / dt * location_constraint;
		auto const lambda = (-inv (K) * (Jvi + stabilization_bias)) / dt;

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
	inline math::SquareMatrix<decltype (1 / 1_kg), N, WorldSpace>
	Constraint::calculate_K (JacobianV<N> const& Jv1,
							 JacobianW<N> const& Jw1,
							 JacobianV<N> const& Jv2,
							 JacobianW<N> const& Jw2) const
	{
		auto const inv_M1 = body_1().frame_cache().inv_M;
		auto const inv_I1 = body_1().frame_cache().inv_I;
		auto const inv_M2 = body_2().frame_cache().inv_M;
		auto const inv_I2 = body_2().frame_cache().inv_I;

		// Unfolded expression: J * inv(M) * ~J.
		// This has to be unfolded because of two distinct scalar types held by Jacobians.
		return Jv1 * inv_M1 * ~Jv1
			 + Jw1 * inv_I1 * ~Jw1
			 + Jv2 * inv_M2 * ~Jv2
			 + Jw2 * inv_I2 * ~Jw2;
	}


template<std::size_t N>
	inline math::SquareMatrix<decltype (1 / 1_kg), N, WorldSpace>
	Constraint::calculate_K (JacobianV<N> const& Jv1,
							 JacobianV<N> const& Jv2) const
	{
		auto const inv_M1 = body_1().frame_cache().inv_M;
		auto const inv_M2 = body_2().frame_cache().inv_M;

		// Unfolded expression: J * inv(M) * ~J.
		return Jv1 * inv_M1 * ~Jv1
			 + Jv2 * inv_M2 * ~Jv2;
	}


template<std::size_t N>
	inline math::SquareMatrix<decltype (1 / 1_kg), N, WorldSpace>
	Constraint::calculate_K (JacobianW<N> const& Jw1,
							 JacobianW<N> const& Jw2) const
	{
		auto const inv_I1 = body_1().frame_cache().inv_I;
		auto const inv_I2 = body_2().frame_cache().inv_I;

		// Unfolded expression: J * inv(M) * ~J.
		return Jw1 * inv_I1 * ~Jw1
			 + Jw2 * inv_I2 * ~Jw2;
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

