/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SHAPES__SHAPE_UTILS_H__INCLUDED
#define XEFIS__SUPPORT__SHAPES__SHAPE_UTILS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/shapes/shape.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Intersect the ray with all shape triangles and return nearest positive hit distance.
 */
template<math::Scalar PositionScalar, math::Scalar DirectionScalar>
	[[nodiscard]]
	std::optional<decltype (PositionScalar{} / DirectionScalar{})>
	ray_shape_intersection (Shape const& shape,
							SpaceVector<PositionScalar, BodyOrigin> const& ray_origin,
							SpaceVector<DirectionScalar, BodyOrigin> const& ray_direction)
	{
		using RayParameter = decltype (PositionScalar{} / DirectionScalar{});

		auto result = std::optional<RayParameter>();

		shape.for_each_triangle ([&] (ShapeVertex const& a, ShapeVertex const& b, ShapeVertex const& c) {
			if (auto const intersection = ray_triangle_intersection (ray_origin, ray_direction, a.position(), b.position(), c.position()))
				if (!result || *intersection < *result)
					result = *intersection;
		});

		return result;
	}

} // namespace xf

#endif
