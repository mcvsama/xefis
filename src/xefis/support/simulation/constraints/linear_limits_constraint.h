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

#ifndef XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__LINEAR_LIMITS_CONSTRAINT_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__LINEAR_LIMITS_CONSTRAINT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/force_moments.h>
#include <xefis/support/simulation/constraints/slider_precomputation.h>
#include <xefis/support/simulation/rigid_body/constraint.h>

// Standard:
#include <cstddef>
#include <optional>


namespace xf::rigid_body {

/**
 * Linear limits, constraints movement between two bodies on given axis.
 */
class LinearLimitsConstraint: public Constraint
{
  public:
	// Ctor
	explicit
	LinearLimitsConstraint (SliderPrecomputation&, std::optional<si::Length> min_distance, std::optional<si::Length> max_distance);

	// Ctor
	explicit
	LinearLimitsConstraint (SliderPrecomputation&, nu::Range<si::Length>);

	/**
	 * Set minimum distance between objects.
	 */
	void
	set_minimum_distance (std::optional<si::Length> const distance)
		{ _min_distance = distance; }

	/**
	 * Set maximum distance between objects.
	 */
	void
	set_maximum_distance (std::optional<si::Length> const distance)
		{ _max_distance = distance; }

	/**
	 * Set minimum and maximum angles.
	 */
	void
	set_distances (std::optional<si::Length> min_distance, std::optional<si::Length> max_distance);

	/**
	 * Set minimum and maximum angles.
	 */
	void
	set_distances (nu::Range<si::Length>);

	// Constraint API
	void
	initialize_step (si::Time dt) override;

  protected:
	// Constraint API
	ConstraintForces
	do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt) override;

  private:
	/**
	 * Return corrective forces for slider limits: minimum distance.
	 */
	std::optional<ConstraintForces>
	min_distance_corrections (VelocityMoments<WorldSpace> const& vm_1,
							  VelocityMoments<WorldSpace> const& vm_2,
							  si::Time dt,
							  SliderPrecomputationData const&) const;

	/**
	 * Return corrective forces for slider limits: maximum distance.
	 */
	std::optional<ConstraintForces>
	max_distance_corrections (VelocityMoments<WorldSpace> const& vm_1,
							  VelocityMoments<WorldSpace> const& vm_2,
							  si::Time dt,
							  SliderPrecomputationData const&) const;

  private:
	SliderPrecomputation&		_slider_precomputation;
	std::optional<si::Length>	_min_distance;
	std::optional<si::Length>	_max_distance;
	JacobianV<1>				_min_Jv1;
	JacobianW<1>				_min_Jw1;
	JacobianV<1>				_min_Jv2;
	JacobianW<1>				_min_Jw2;
	ConstraintZMatrix<1>		_min_Z;
	LocationConstraint<1>		_min_location_constraint_value;
	JacobianV<1>				_max_Jv1;
	JacobianW<1>				_max_Jw1;
	JacobianV<1>				_max_Jv2;
	JacobianW<1>				_max_Jw2;
	ConstraintZMatrix<1>		_max_Z;
	LocationConstraint<1>		_max_location_constraint_value;
};


inline void
LinearLimitsConstraint::set_distances (std::optional<si::Length> const min_distance, std::optional<si::Length> const max_distance)
{
	set_minimum_distance (min_distance);
	set_maximum_distance (max_distance);
}


inline void
LinearLimitsConstraint::set_distances (nu::Range<si::Length> const range)
{
	set_distances (range.min(), range.max());
}

} // namespace xf::rigid_body

#endif

