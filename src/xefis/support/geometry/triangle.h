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

#ifndef XEFIS__SUPPORT__GEOMETRY__TRIANGLES_H__INCLUDED
#define XEFIS__SUPPORT__GEOMETRY__TRIANGLES_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

template<class Scalar, class Space>
	inline PlaneVector<Scalar, Space>
	triangle_centroid (PlaneVector<Scalar, Space> const& a,
					   PlaneVector<Scalar, Space> const& b,
					   PlaneVector<Scalar, Space> const& c)
	{
		return 1.0 / 3 * (a + b + c);
	}


template<class Scalar, class Space>
	inline PlaneVector<Scalar, Space>
	triangle_centroid (PlaneTriangle<Scalar, Space> const& triangle)
	{
		return triangle_centroid (triangle[0], triangle[1], triangle[2]);
	}

} // namespace xf

#endif

