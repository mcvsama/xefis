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

#ifndef XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__HINGE_PRECALCULATION_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__HINGE_PRECALCULATION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/constraints/helpers/fixed_orientation_helper.h>
#include <xefis/support/simulation/rigid_body/frame_precalculation.h>
#include <xefis/support/nature/force_moments.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

class HingePrecalculationData
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
	// Normalized hinge visible from both bodies:
	SpaceVector<double, WorldSpace>		a1;
	SpaceVector<double, WorldSpace>		a2;
	// Two vectors orthogonal to a1 and to each other:
	SpaceLength<WorldSpace>				t1;
	SpaceLength<WorldSpace>				t2;
	// Angle between the two bodies:
	si::Angle							angle;
};


class HingePrecalculation: public FramePrecalculation<HingePrecalculationData>
{
  private:
	/**
	 * Common initializer for all other ctors.
	 */
	explicit
	HingePrecalculation (Body& body_1, Body& body_2);

  public:
	/**
	 * Create a hinge between two bodies.
	 * Hinge vector/axis is given relative to the first body.
	 */
	explicit
	HingePrecalculation (SpaceLength<BodySpace> const& anchor_point_1,
						 SpaceLength<BodySpace> const& anchor_point_2,
						 Body& body_1,
						 Body& body_2);

	/**
	 * Create a hinge between two bodies.
	 * Hinge vector/axis is given relative to the second body.
	 */
	explicit
	HingePrecalculation (Body& body_1,
						 Body& body_2,
						 SpaceLength<BodySpace> const& anchor_point_1,
						 SpaceLength<BodySpace> const& anchor_point_2);

	/**
	 * Create a hinge between two bodies.
	 * Hinge vector/axis is given in world space coordinates.
	 */
	explicit
	HingePrecalculation (Body& body_1,
						 SpaceLength<WorldSpace> const& anchor_point_1,
						 SpaceLength<WorldSpace> const& anchor_point_2,
						 Body& body_2);

	// Ctor
	explicit
	HingePrecalculation (HingePrecalculation const&) = default;

	/**
	 * Return anchor as visible from the first body.
	 */
	SpaceLength<BodySpace> const&
	body_1_anchor() const noexcept
		{ return _anchor_1; }

	/**
	 * Return anchor as visible from the second body.
	 */
	SpaceLength<BodySpace> const&
	body_2_anchor() const noexcept
		{ return _anchor_2; }

	/**
	 * Return hinge as visible from the first body.
	 */
	SpaceLength<BodySpace> const&
	body_1_hinge() const noexcept
		{ return _hinge_1; }

	/**
	 * Return hinge as visible from the second body.
	 */
	SpaceLength<BodySpace> const&
	body_2_hinge() const noexcept
		{ return _hinge_2; }

  protected:
	// FramePrecalculation API
	void
	calculate (HingePrecalculationData&) override;

  private:
	SpaceLength<BodySpace>	_anchor_1;
	SpaceLength<BodySpace>	_anchor_2;
	SpaceLength<BodySpace>	_hinge_1;
	SpaceLength<BodySpace>	_hinge_2;
	FixedOrientationHelper	_fixed_orientation;
};

} // namespace xf::rigid_body

#endif

