/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
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
#include "gl_space.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>
#include <optional>


namespace xf {

void
GLSpace::set_hfov_perspective (QSize const size, si::Angle hfov, float near_plane, float far_plane)
{
	auto const aspect = 1.0 * size.width() / size.height();

	auto const tangent = tan (0.5 * hfov);
	auto const height = near_plane * tangent;
	auto const width = height * aspect;

	glFrustum (-width, width, -height, height, near_plane, far_plane);
}


rigid_body::ShapeMaterial
GLSpace::make_material (QColor const& color)
{
	rigid_body::ShapeMaterial m;
	m.set_diffuse_color (color);
	m.set_ambient_color (color);
	m.set_specular_color (color);
	m.set_shininess (0.1);
	return m;
}


void
GLSpace::set_material (rigid_body::ShapeMaterial const& material)
{
	glFogCoordf (material.fog_distance());
	glColor4fv (to_opengl (material.emission_color()));
	glMaterialfv (GL_FRONT, GL_EMISSION, to_opengl (material.emission_color()));
	glMaterialfv (GL_FRONT, GL_AMBIENT, to_opengl (material.ambient_color()));
	glMaterialfv (GL_FRONT, GL_DIFFUSE, to_opengl (material.diffuse_color()));
	glMaterialfv (GL_FRONT, GL_SPECULAR, to_opengl (material.specular_color()));
	glMaterialf (GL_FRONT, GL_SHININESS, material.shininess() * 127);
}


void
GLSpace::draw (rigid_body::Shape const& shape)
{
	save_matrix ([&] {
		if (!shape.triangles().empty())
		{
			for (auto const& triangle: shape.triangles())
			{
				begin (GL_TRIANGLES, [&] {
					for (auto const& vertex: triangle)
						add_vertex (vertex);
				});
			}
		}

		if (!shape.triangle_strips().empty())
		{
			for (auto const& strip: shape.triangle_strips())
			{
				begin (GL_TRIANGLE_STRIP, [&] {
					for (auto const& vertex: strip)
						add_vertex (vertex);
				});
			}
		}

		if (!shape.triangle_fans().empty())
		{
			for (auto const& fan: shape.triangle_fans())
			{
				begin (GL_TRIANGLE_FAN, [&] {
					for (auto const& vertex: fan)
						add_vertex (vertex);
				});
			}
		}
	});
}

} // namespace xf

