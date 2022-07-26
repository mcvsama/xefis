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

// Local:
#include "slider_precalculation.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

SliderPrecalculation::SliderPrecalculation (Body& body_1,
											Body& body_2,
											SpaceVector<double, WorldSpace> const& axis):
	FramePrecalculation (body_1, body_2),
	_fixed_orientation (body_1.location(), body_2.location())
{
	auto const loc_1 = body_1.location();
	auto const loc_2 = body_2.location();

	// Choose anchor point at world origin (it can be anything). Compute and save two relative vectors to it.
	SpaceLength<WorldSpace> const origin (math::zero);
	_anchor_1 = loc_1.bound_transform_to_body (origin);
	_anchor_2 = loc_2.bound_transform_to_body (origin);
	_axis_1 = loc_1.unbound_transform_to_body (axis);
	_axis_2 = loc_2.unbound_transform_to_body (axis);
}


void
SliderPrecalculation::calculate (SliderPrecalculationData& data)
{
	auto const loc_1 = body_1().location();
	auto const loc_2 = body_2().location();

	auto const x1 = loc_1.position();
	auto const x2 = loc_2.position();
	auto const r1 = loc_1.unbound_transform_to_base (_anchor_1);
	auto const r2 = loc_2.unbound_transform_to_base (_anchor_2);
	auto const u = x2 + r2 - x1 - r1;
	auto const a1 = loc_1.unbound_transform_to_base (_axis_1);
	auto const distance = (~u * a1).scalar();
	auto const z = find_non_colinear (a1);
	auto const t1 = normalized (cross_product (a1, z));
	auto const t2 = normalized (cross_product (a1, t1));

	data.x1 = x1;
	data.x2 = x2;
	data.r1 = r1;
	data.r2 = r2;
	data.u = u;
	data.a = a1;
	data.t1 = t1;
	data.t2 = t2;
	data.distance = distance;
	// Used by limits:
	data.r1uxa = ~cross_product (r1 + u, a1);
	data.r2xa = ~cross_product (r2, a1);
	// Angular differences:
	data.rotation_error = _fixed_orientation.rotation_constraint_value (loc_1, loc_2);
}

} // namespace xf::rigid_body

