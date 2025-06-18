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

#ifndef XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__HINGE_PRECOMPUTATION_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__HINGE_PRECOMPUTATION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/constraints/helpers/fixed_orientation_helper.h>
#include <xefis/support/simulation/rigid_body/frame_precomputation.h>
#include <xefis/support/nature/force_moments.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

class HingePrecomputationData
{
  public:
	// Body 1 and 2 positions:
	SpaceLength<WorldSpace>				x1;
	SpaceLength<WorldSpace>				x2;
	// Vectors from bodies to anchor point:
	SpaceLength<WorldSpace>				r1;
	SpaceLength<WorldSpace>				r2;
	// x2 + r2 - x1 - r1:
	SpaceLength<WorldSpace>				u;
	// Normalized hinge axis visible from both bodies:
	SpaceVector<double, WorldSpace>		a1;
	SpaceVector<double, WorldSpace>		a2;
	// Two vectors orthogonal to a1 and to each other:
	SpaceLength<WorldSpace>				t1;
	SpaceLength<WorldSpace>				t2;
	// Angle between the two bodies:
	si::Angle							angle;
};


class HingePrecomputation: public FramePrecomputation<HingePrecomputationData>
{
  private:
	/**
	 * Common initializer for all other ctors.
	 */
	explicit
	HingePrecomputation (Body& body_1, Body& body_2);

  public:
	/**
	 * Create a hinge between two bodies.
	 * Hinge vector/axis is given relative to the first body.
	 */
	explicit
	HingePrecomputation (SpaceLength<BodyCOM> const& anchor_point_1,
						 SpaceLength<BodyCOM> const& anchor_point_2,
						 Body& body_1,
						 Body& body_2);

	/**
	 * Create a hinge between two bodies.
	 * Hinge vector/axis is given relative to the second body.
	 */
	explicit
	HingePrecomputation (Body& body_1,
						 Body& body_2,
						 SpaceLength<BodyCOM> const& anchor_point_1,
						 SpaceLength<BodyCOM> const& anchor_point_2);

	/**
	 * Create a hinge between two bodies.
	 * Hinge vector/axis is given in world space coordinates.
	 */
	explicit
	HingePrecomputation (Body& body_1,
						 SpaceLength<WorldSpace> const& anchor_point_1,
						 SpaceLength<WorldSpace> const& anchor_point_2,
						 Body& body_2);

	// Ctor
	explicit
	HingePrecomputation (HingePrecomputation const&) = default;

	/**
	 * Return anchor as visible from the first body.
	 */
	SpaceLength<BodyCOM> const&
	body_1_anchor() const noexcept
		{ return _anchor_1; }

	/**
	 * Return anchor as visible from the second body.
	 */
	SpaceLength<BodyCOM> const&
	body_2_anchor() const noexcept
		{ return _anchor_2; }

	/**
	 * Return hinge as visible from the first body.
	 */
	SpaceLength<BodyCOM> const&
	body_1_hinge() const noexcept
		{ return _hinge_1; }

	/**
	 * Return hinge as visible from the second body.
	 */
	SpaceLength<BodyCOM> const&
	body_2_hinge() const noexcept
		{ return _hinge_2; }

	/**
	 * Return hinge as visible from the first body.
	 */
	SpaceLength<BodyCOM> const&
	body_1_normalized_hinge() const noexcept
		{ return _normalized_hinge_1; }

	/**
	 * Return hinge as visible from the second body.
	 */
	SpaceLength<BodyCOM> const&
	body_2_normalized_hinge() const noexcept
		{ return _normalized_hinge_2; }

  protected:
	// FramePrecomputation API
	void
	compute (HingePrecomputationData&) override;

  private:
	// Anchor as visible from each body:
	SpaceLength<BodyCOM>	_anchor_1;
	SpaceLength<BodyCOM>	_anchor_2;
	// Hinge as visible from each body:
	SpaceLength<BodyCOM>	_hinge_1;
	SpaceLength<BodyCOM>	_hinge_2;
	SpaceLength<BodyCOM>	_normalized_hinge_1;
	SpaceLength<BodyCOM>	_normalized_hinge_2;
	FixedOrientationHelper	_fixed_orientation;
};

} // namespace xf::rigid_body

#endif

