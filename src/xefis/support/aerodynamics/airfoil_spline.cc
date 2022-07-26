/* vim:ts=4
 *
 * Copyleft 2012…2018  Michał Gawron
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
#include "airfoil_spline.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>
#include <limits>


namespace xf {

AirfoilSpline::AirfoilSpline (std::initializer_list<Point> points):
	_points (points)
{ }


AirfoilSpline::AirfoilSpline (std::vector<Point> const& points):
	_points (points)
{ }


std::pair<double, double>
AirfoilSpline::projected_chord_and_thickness (si::Angle const alpha, si::Angle const beta) const
{
	double min_x = std::numeric_limits<double>::max();
	double max_x = std::numeric_limits<double>::min();
	double min_y = std::numeric_limits<double>::max();
	double max_y = std::numeric_limits<double>::min();

	for (auto point: _points)
	{
		auto rotated = z_rotation<AirfoilSplineSpace> (alpha) * Point::Resized<1, 3> { point[0], point[1], 0.0 };
		// Get shadow on the X-plane:
		min_x = std::min (min_x, rotated[0]);
		max_x = std::max (max_x, rotated[0]);
		// Get shadow on the Y-plane:
		min_y = std::min (min_y, rotated[1]);
		max_y = std::max (max_y, rotated[1]);
	}

	auto const len_x = max_x - min_x;
	auto const len_y = max_y - min_y;

	return {
		std::abs (cos (beta) * len_x),
		std::abs (cos (beta) * len_y),
	};
}

} // namespace xf

