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

#ifndef XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__SLIDER_PRECALCULATION_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__SLIDER_PRECALCULATION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/constraints/helpers/fixed_orientation_helper.h>
#include <xefis/support/simulation/rigid_body/frame_precalculation.h>
#include <xefis/support/nature/force_moments.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

class SliderPrecalculationData
{
  public:
	// Body 1 and 2 positions:
	SpaceLength<WorldSpace>						x1;
	SpaceLength<WorldSpace>						x2;
	// Vectors from bodies to anchor point:
	SpaceLength<WorldSpace>						r1;
	SpaceLength<WorldSpace>						r2;
	// x2 + r2 - x1 - r1:
	SpaceLength<WorldSpace>						u;
	// Axis vector:
	SpaceVector<double, WorldSpace>				a;
	// Two vectors orthogonal to a and to each other:
	SpaceVector<double, WorldSpace>				t1;
	SpaceVector<double, WorldSpace>				t2;
	// Distance:
	si::Length									distance;
	// Used when limits are on:
	SpaceLength<WorldSpace>::TransposedMatrix	r1uxa;
	SpaceLength<WorldSpace>::TransposedMatrix	r2xa;
	// Angular differences:
	SpaceLength<WorldSpace>						rotation_error;
};


class SliderPrecalculation: public FramePrecalculation<SliderPrecalculationData>
{
  public:
	// Ctor
	explicit
	SliderPrecalculation (Body& body_1,
						  Body& body_2,
						  SpaceVector<double, WorldSpace> const& axis);

	// Ctor
	explicit
	SliderPrecalculation (SliderPrecalculation const&) = default;

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
	SpaceVector<double, BodySpace> const&
	body_1_axis() const noexcept
		{ return _axis_1; }

	/**
	 * Return hinge as visible from the second body.
	 */
	SpaceVector<double, BodySpace> const&
	body_2_axis() const noexcept
		{ return _axis_2; }

  protected:
	// FramePrecalculation API
	void
	calculate (SliderPrecalculationData&) override;

  private:
	SpaceLength<BodySpace>			_anchor_1;
	SpaceLength<BodySpace>			_anchor_2;
	SpaceVector<double, BodySpace>	_axis_1;
	SpaceVector<double, BodySpace>	_axis_2;
	FixedOrientationHelper			_fixed_orientation;
};

} // namespace xf::rigid_body

#endif

