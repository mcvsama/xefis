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

#ifndef XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__ANGULAR_SPRING_CONSTRAINT_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__ANGULAR_SPRING_CONSTRAINT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/constraints/hinge_precalculation.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/simulation/rigid_body/constraint.h>

// Standard:
#include <cstddef>
#include <memory>


namespace xf::rigid_body {

using TorqueForAngle = decltype (1_Nm / 1_rad);


/**
 * Angular spring constraint. Generates forces relative to angular displacement of two bodies about
 * a configured axis.
 */
class AngularSpringConstraint: public Constraint
{
  public:
	/**
	 * Returns Torque for angular displacement. Preferred positive torque for positive angle gives negative feedback,
	 * which stabilizes the constraint.
	 */
	using SpringTorqueFunction = std::function<si::Torque (si::Angle, SpaceVector<double, WorldSpace> const& hinge)>;

  public:
	// Ctor
	explicit
	AngularSpringConstraint (HingePrecalculation&, SpringTorqueFunction);

	// Constraint API
	ConstraintForces
	do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
						  VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
						  si::Time dt) override;

  private:
	HingePrecalculation&	_hinge;
	SpringTorqueFunction	_spring_torque;
};


/*
 * Global functions
 */


/**
 * Return an action torque function of angular error.
 */
constexpr auto
angular_spring_function (TorqueForAngle torque_for_angle)
{
	return [=] (si::Angle angle, SpaceVector<double, WorldSpace> const&) {
		return torque_for_angle * angle;
	};
}

} // namespace xf::rigid_body

#endif

