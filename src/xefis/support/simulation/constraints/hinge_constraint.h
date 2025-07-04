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
#include <xefis/support/simulation/constraints/hinge_precomputation.h>
#include <xefis/support/simulation/rigid_body/constraint.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

class HingeConstraint: public Constraint
{
  public:
	// Ctor
	explicit
	HingeConstraint (HingePrecomputation&);

	/**
	 * Return reference to HingePrecomputation.
	 */
	HingePrecomputation const&
	hinge_precomputation() const noexcept
		{ return _hinge_precomputation; }

	// Constraint API
	void
	initialize_step (si::Time dt) override;

  protected:
	// Constraint API
	ConstraintForces
	do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt) override;

  private:
	HingePrecomputation&	_hinge_precomputation;
	JacobianV<5>			_Jv1;
	JacobianW<5>			_Jw1;
	JacobianV<5>			_Jv2;
	JacobianW<5>			_Jw2;
	ConstraintZMatrix<5>	_Z;
	LocationConstraint<5>	_location_constraint_value;
};

} // namespace xf::rigid_body

#endif

