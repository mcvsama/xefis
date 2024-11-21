/* vim:ts=4
 *
 * Copyleft 2023  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__ANGULAR_MOTOR_CONSTRAINT_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__ANGULAR_MOTOR_CONSTRAINT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/force_moments.h>
#include <xefis/support/simulation/constraints/hinge_precalculation.h>
#include <xefis/support/simulation/rigid_body/constraint.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

class AngularMotorConstraint: public Constraint
{
  public:
	// Ctor
	explicit
	AngularMotorConstraint (HingePrecalculation&, si::AngularVelocity max_angular_velocity = 0_radps, si::Torque const torque = 0_Nm);

	/**
	 * Torque used to move the motor. Always positive.
	 */
	[[nodiscard]]
	si::Torque
	torque() const noexcept
		{ return _force * 1_m; }

	/**
	 * Set torque used to move the motor. Must always be positive.
	 * The direction motion is set with max_angular_velocity().
	 */
	void
	set_abs_torque (si::Torque const torque) noexcept
		{ _force = torque / 1_m; }

	/**
	 * Max angular velocity, positive or negative, depending on wanted direction.
	 */
	[[nodiscard]]
	si::AngularVelocity
	max_angular_velocity() const noexcept
		{ return _max_angular_velocity; }

	/**
	 * Set max angular velocity, positive or negative, depending on wanted direction.
	 */
	void
	set_max_angular_velocity (si::AngularVelocity const w) noexcept
		{ _max_angular_velocity = w; }

  protected:
	// Constraint API
	ConstraintForces
	do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt) const override;

  private:
	HingePrecalculation&	_hinge_precalculation;
	si::AngularVelocity		_max_angular_velocity;
	// Even though it's a torque, it's more convenient to keep it as a force:
	si::Force				_force;
};

} // namespace xf::rigid_body

#endif

