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


namespace xf::rigid_body {

void
Shape::rotate (RotationMatrix<BodySpace> const& rotation)
{
	for_all_vertices ([&] (ShapeVertex& vertex) {
		vertex.rotate (rotation);
	});
}


void
Shape::translate (SpaceLength<BodySpace> const& translation)
{
	for_all_vertices ([&] (ShapeVertex& vertex) {
		vertex.translate (translation);
	});
}


void
Shape::for_all_vertices (std::function<void (ShapeVertex&)> const vertex_function)
{
	for (auto& geometry: _triangles)
		for (auto& vertex: geometry)
			vertex_function (vertex);

	for (auto& geometry: _triangle_strips)
		for (auto& vertex: geometry)
			vertex_function (vertex);

	for (auto& geometry: _triangle_fans)
		for (auto& vertex: geometry)
			vertex_function (vertex);
}

} // namespace xf::rigid_body

