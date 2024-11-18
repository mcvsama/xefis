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
#include "hinge_precalculation.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>
#include <cmath>


namespace xf::rigid_body {

HingePrecalculation::HingePrecalculation (Body& body_1, Body& body_2):
	FramePrecalculation (body_1, body_2),
	_fixed_orientation (body_1.placement(), body_2.placement())
{ }


HingePrecalculation::HingePrecalculation (SpaceLength<BodyCOM> const& anchor_point_1,
										  SpaceLength<BodyCOM> const& anchor_point_2,
										  Body& body_1,
										  Body& body_2):
	HingePrecalculation (body_1, body_2)
{
	auto const pl_1 = body_1.placement();
	auto const pl_2 = body_2.placement();

	_anchor_1 = anchor_point_1;
	_anchor_2 = pl_2.bound_transform_to_body (pl_1.bound_transform_to_base (anchor_point_1));
	_hinge_1 = anchor_point_2 - anchor_point_1;
	_hinge_2 = pl_2.unbound_transform_to_body (pl_1.unbound_transform_to_base (_hinge_1));
	_normalized_hinge_1 = _hinge_1.normalized();
	_normalized_hinge_2 = _hinge_2.normalized();
}


HingePrecalculation::HingePrecalculation (Body& body_1,
										  Body& body_2,
										  SpaceLength<BodyCOM> const& anchor_point_1,
										  SpaceLength<BodyCOM> const& anchor_point_2):
	HingePrecalculation (body_1, body_2)
{
	auto const pl_1 = body_1.placement();
	auto const pl_2 = body_2.placement();

	_anchor_1 = pl_1.bound_transform_to_body (pl_2.bound_transform_to_base (anchor_point_1));
	_anchor_2 = anchor_point_1;
	_hinge_1 = anchor_point_2 - anchor_point_1;
	_hinge_2 = pl_2.unbound_transform_to_body (pl_1.unbound_transform_to_base (_hinge_1));
	_normalized_hinge_1 = _hinge_1.normalized();
	_normalized_hinge_2 = _hinge_2.normalized();
}


HingePrecalculation::HingePrecalculation (Body& body_1,
										  SpaceLength<WorldSpace> const& anchor_point_1,
										  SpaceLength<WorldSpace> const& anchor_point_2,
										  Body& body_2):
	HingePrecalculation (body_1, body_2)
{
	auto const pl_1 = body_1.placement();
	auto const pl_2 = body_2.placement();
	auto const hinge = anchor_point_2 - anchor_point_1;

	_anchor_1 = pl_1.bound_transform_to_body (anchor_point_1);
	_anchor_2 = pl_2.bound_transform_to_body (anchor_point_2);
	_hinge_1 = pl_1.unbound_transform_to_body (hinge);
	_hinge_2 = pl_2.unbound_transform_to_body (hinge);
	_normalized_hinge_1 = _hinge_1.normalized();
	_normalized_hinge_2 = _hinge_2.normalized();
}


void
HingePrecalculation::calculate (HingePrecalculationData& data)
{
	auto const pl_1 = body_1().placement();
	auto const pl_2 = body_2().placement();
	auto const x1 = body_1().placement().position();
	auto const x2 = body_2().placement().position();
	auto const r1 = pl_1.unbound_transform_to_base (body_1_anchor());
	auto const r2 = pl_2.unbound_transform_to_base (body_2_anchor());
	auto const u = x2 + r2 - x1 - r1;
	auto const a1 = pl_1.unbound_transform_to_base (body_1_normalized_hinge()) / abs (body_1_normalized_hinge());
	auto const a2 = pl_2.unbound_transform_to_base (body_2_normalized_hinge()) / abs (body_2_normalized_hinge());
	auto const t1 = cross_product (a1, find_non_colinear (a1) * 1_m).normalized();
	auto const t2 = cross_product (a1, t1).normalized();
	auto const rotation_error = _fixed_orientation.rotation_constraint_value (pl_1, pl_2);
	auto const angle = projection_onto_normalized (rotation_error, a1);

	data.x1 = x1;
	data.x2 = x2;
	data.r1 = r1;
	data.r2 = r2;
	data.u = u;
	data.a1 = a1;
	data.a2 = a2;
	data.t1 = t1;
	data.t2 = t2;
	// Used by angular limits:
	data.angle = std::copysign (abs (angle) / 1_m, (~angle * a1).scalar() / 1_m) * 1_rad;
}

} // namespace xf::rigid_body

