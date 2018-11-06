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

#ifndef XEFIS__SUPPORT__SIMULATION__AIRFOIL_SPLINE_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__AIRFOIL_SPLINE_H__INCLUDED

// Standard:
#include <cstddef>
#include <initializer_list>
#include <vector>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/space.h>


namespace xf {

class AirfoilSpline
{
  public:
	using Point = PlaneVector<double, AirfoilSplineFrame>;

  public:
	// Ctor
	explicit
	AirfoilSpline (std::initializer_list<Point>);

	// Ctor
	explicit
	AirfoilSpline (std::vector<Point> const&);

	/**
	 * Return airfoil chord length projected onto plane defined by wind vector and airfoil thickness projected onto plane defined by lift vector.
	 * Used to compute areas in the airfoil lift and drag equations.
	 */
	[[nodiscard]]
	PlaneVector<>
	projected_chord_and_thickness (si::Angle alpha, si::Angle beta) const;

  private:
	std::vector<Point> _points;
};

} // namespace xf

#endif

