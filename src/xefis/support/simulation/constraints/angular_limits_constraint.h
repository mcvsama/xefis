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

#ifndef XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__ANGULAR_LIMITS_CONSTRAINT_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__ANGULAR_LIMITS_CONSTRAINT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/force_moments.h>
#include <xefis/support/simulation/constraints/hinge_precalculation.h>
#include <xefis/support/simulation/rigid_body/constraint.h>

// Standard:
#include <cstddef>
#include <optional>


namespace xf::rigid_body {

/**
 * Angular limits, constraints angle between two bodies.
 * Uses HingePrecalculation to define the two bodies and the hinge about which angle will be measured.
 */
class AngularLimitsConstraint: public Constraint
{
  public:
	// Ctor
	explicit
	AngularLimitsConstraint (HingePrecalculation&, std::optional<si::Angle> min_angle, std::optional<si::Angle> max_angle);

	// Ctor
	explicit
	AngularLimitsConstraint (HingePrecalculation&, Range<si::Angle>);

	/**
	 * Set minimum hinge angle.
	 */
	void
	set_minimum_angle (std::optional<si::Angle> const angle)
		{ _min_angle = angle; }

	/**
	 * Set maximum hinge angle.
	 */
	void
	set_maximum_angle (std::optional<si::Angle> const angle)
		{ _max_angle = angle; }

	/**
	 * Set minimum and maximum angles.
	 */
	void
	set_angles (std::optional<si::Angle> min_angle, std::optional<si::Angle> max_angle);

	/**
	 * Set minimum and maximum angles.
	 */
	void
	set_angles (Range<si::Angle>);

	// Constraint API
	ConstraintForces
	do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
						  VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
						  si::Time dt) override;

  private:
	/**
	 * Return corrective forces for hinge limits: minimum angle.
	 */
	std::optional<ConstraintForces>
	min_angle_corrections (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
						   VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
						   si::Time dt,
						   HingePrecalculationData const&) const;

	/**
	 * Return corrective forces for hinge limits: maximum angle.
	 */
	std::optional<ConstraintForces>
	max_angle_corrections (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
						   VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
						   si::Time dt,
						   HingePrecalculationData const&) const;

  private:
	HingePrecalculation&		_hinge_precalculation;
	std::optional<si::Angle>	_min_angle;
	std::optional<si::Angle>	_max_angle;
};


inline void
AngularLimitsConstraint::set_angles (std::optional<si::Angle> const min_angle, std::optional<si::Angle> const max_angle)
{
	set_minimum_angle (min_angle);
	set_maximum_angle (max_angle);
}


inline void
AngularLimitsConstraint::set_angles (Range<si::Angle> const range)
{
	set_angles (range.min(), range.max());
}

} // namespace xf::rigid_body

#endif

