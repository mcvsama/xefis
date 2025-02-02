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

GLSpace::GLSpace (decltype (1 / 1_m) position_scale):
	_position_scale (position_scale)
{
	_additional_parameters_stack.push({});
}


void
GLSpace::set_hfov_perspective (QSize const size, si::Angle hfov, float near_plane, float far_plane)
{
	auto const aspect = 1.0 * size.width() / size.height();

	auto const tangent = tan (0.5 * hfov);
	auto const height = near_plane * tangent;
	auto const width = height * aspect;

	glFrustum (-width, width, -height, height, near_plane, far_plane);
}


void
GLSpace::set_material (rigid_body::ShapeMaterial const& material)
{
	auto const& params = additional_parameters();

	glFogCoordf (material.gl_fog_distance);
	glColor4fv (material.gl_emission_color);
	glMaterialf (GL_FRONT, GL_SHININESS, material.gl_shininess);
	glMaterialfv (GL_FRONT, GL_EMISSION, material.gl_emission_color);

	if (params.color_override)
	{
		glMaterialfv (GL_FRONT, GL_AMBIENT, *params.color_override);
		glMaterialfv (GL_FRONT, GL_DIFFUSE, *params.color_override);
		glMaterialfv (GL_FRONT, GL_SPECULAR, *params.color_override);
	}
	else
	{
		glMaterialfv (GL_FRONT, GL_AMBIENT, material.gl_ambient_color);
		glMaterialfv (GL_FRONT, GL_DIFFUSE, material.gl_diffuse_color);
		glMaterialfv (GL_FRONT, GL_SPECULAR, material.gl_specular_color);
	}
}


void
GLSpace::draw (rigid_body::Shape const& shape)
{
	save_context ([&] {
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


void
GLSpace::clear_z_buffer (float const value)
{
	glClearDepth (value);
	glClear (GL_DEPTH_BUFFER_BIT);
}


void
GLSpace::push_context()
{
	if (_additional_parameters_stack.empty())
		_additional_parameters_stack.push ({});
	else
		_additional_parameters_stack.push (_additional_parameters_stack.top());

	glPushMatrix();
}


void
GLSpace::pop_context()
{
	glPopMatrix();

	if (!_additional_parameters_stack.empty())
		_additional_parameters_stack.pop();
}


GLSpace::AdditionalParameters&
GLSpace::additional_parameters()
{
	if (_additional_parameters_stack.empty())
		_additional_parameters_stack.push ({});

	return _additional_parameters_stack.top();
}

} // namespace xf

