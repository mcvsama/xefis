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

#ifndef XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__HINGE_CONSTRAINT_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__HINGE_CONSTRAINT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/force_moments.h>
#include <xefis/support/simulation/constraints/hinge_precalculation.h>
#include <xefis/support/simulation/rigid_body/constraint.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

class HingeConstraint: public Constraint
{
  public:
	// Ctor
	explicit
	HingeConstraint (HingePrecalculation&);

	/**
	 * Return reference to HingePrecalculation.
	 */
	HingePrecalculation const&
	hinge_precalculation() const noexcept
		{ return _hinge_precalculation; }

	// Constraint API
	ConstraintForces
	do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
						  VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
						  si::Time dt) override;

  private:
	HingePrecalculation& _hinge_precalculation;
};

} // namespace xf::rigid_body

#endif

