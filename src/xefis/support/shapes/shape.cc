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
#include "shape.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

void
Shape::transform (AffineTransform<BodyOrigin> const& transform)
{
	for_all_vertices ([&] (ShapeVertex& vertex) {
		vertex.transform (transform);
	});
}


void
Shape::rotate (RotationQuaternion<BodyOrigin> const& rotation)
{
	for_all_vertices ([&] (ShapeVertex& vertex) {
		vertex.rotate (rotation);
	});
}


void
Shape::translate (SpaceLength<BodyOrigin> const& translation)
{
	for_all_vertices ([&] (ShapeVertex& vertex) {
		vertex.translate (translation);
	});
}


void
Shape::for_all_vertices (std::function<void (ShapeVertex&)> const vertex_function)
{
	for (auto* geometries: { &_triangles, &_triangle_strips, &_triangle_fans, &_quads })
		for (auto& geometry: *geometries)
			for (auto& vertex: geometry.vertices)
				vertex_function (vertex);
}

} // namespace xf

