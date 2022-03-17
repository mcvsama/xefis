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

#ifndef XEFIS__SUPPORT__AERODYNAMICS__AIRFOIL_SPLINE_H__INCLUDED
#define XEFIS__SUPPORT__AERODYNAMICS__AIRFOIL_SPLINE_H__INCLUDED

// Standard:
#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/geometry/triangulation.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/nature/mass_moments.h>


namespace xf {

/**
 * Points defining an airfoil lie on X-Y plane, where X is parallel to the airfoil's chord,
 * positive X is at the trailing edge, positive Y is at the top of the airfoil.
 */
struct AirfoilSplineSpace;


/**
 * Represents an airfoil spline.
 * The points are listed in counter-clockwise direction.
 */
class AirfoilSpline
{
  public:
	using Point = PlaneVector<double, AirfoilSplineSpace>;

  public:
	/**
	 * Create a spline.
	 *
	 * \param	points
	 *			The X range of all points must fit within 0…1 range. They also must be listed
	 *			in counter-clockwise direction.
	 */
	explicit
	AirfoilSpline (std::initializer_list<Point> points);

	/**
	 * Create a spline.
	 *
	 * \param	points
	 *			The X range of all points must fit within 0…1 range. They also must be listed
	 *			in counter-clockwise direction.
	 */
	explicit
	AirfoilSpline (std::vector<Point> const& points);

	/**
	 * Return spline points.
	 */
	std::vector<Point> const&
	points() const noexcept
		{ return _points; }

	/**
	 * Return airfoil chord length projected onto plane defined by wind vector and airfoil thickness projected onto plane defined by lift vector.
	 * Used to compute areas in the airfoil lift and drag equations.
	 */
	[[nodiscard]]
	std::pair<double, double>
	projected_chord_and_thickness (si::Angle alpha, si::Angle beta) const;

  private:
	std::vector<Point> _points;
};


/*
 * Global functions
 */


template<class Space>
	MassMoments<Space>
	calculate_mass_moments (AirfoilSpline const& airfoil_spline, si::Length const chord_length, si::Length const wing_length, si::Density const material_density)
	{
		auto const triangulation = triangulate<double, Space> (begin (airfoil_spline.points()), end (airfoil_spline.points()));

		return calculate_mass_moments (triangulation, chord_length, wing_length, material_density);
	}

} // namespace xf

#endif

