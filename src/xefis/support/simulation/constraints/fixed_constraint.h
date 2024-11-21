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

#ifndef XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__FIXED_CONSTRAINT_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__FIXED_CONSTRAINT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/force_moments.h>
#include <xefis/support/simulation/constraints/helpers/fixed_orientation_helper.h>
#include <xefis/support/simulation/rigid_body/constraint.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

class FixedConstraint: public Constraint
{
  public:
	/**
	 * Create a fixed constraint between two bodies.
	 *
	 * \param	body_1, body_2
	 *			References to connected bodies. Constraint must not outlive bodys.
	 */
	explicit
	FixedConstraint (Body& body_1, Body& body_2);

  protected:
	// Constraint API
	ConstraintForces
	do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt) const override;

  private:
	SpaceLength<BodyCOM>	_anchor_1;
	SpaceLength<BodyCOM>	_anchor_2;
	FixedOrientationHelper	_fixed_orientation;
	JacobianV<6> mutable			_Jv1;
	JacobianW<6> mutable			_Jw1;
	JacobianV<6> mutable			_Jv2;
	JacobianW<6> mutable			_Jw2;
};

} // namespace xf::rigid_body

#endif

