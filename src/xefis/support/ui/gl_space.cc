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

// System:
#include <GL/glext.h>

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
GLSpace::set_camera (std::optional<Placement<WorldSpace, WorldSpace>> const& camera)
{
	_camera = camera;
	load_identity();

	// Apply camera rotation (inverse, that is base→body, since OpenGL rotates the world, not the objects):
	if (_camera)
		rotate (_camera->body_rotation());

	// Don't translate now. The GL matrix has 32-bit floats, we need to apply translation when applying verices using 64-bit floats (si::Length).
}


void
GLSpace::set_camera_rotation_only (Placement<WorldSpace, WorldSpace> camera)
{
	camera.set_position (math::zero);
	set_camera (camera);
}


void
GLSpace::reset_translation()
{
	GLfloat matrix[16];
	glGetFloatv (GL_MODELVIEW_MATRIX, matrix);

	// Remove translation (set the last column’s x, y, z to 0):
	matrix[12] = 0.0f; // X translation
	matrix[13] = 0.0f; // Y translation
	matrix[14] = 0.0f; // Z translation

	glMatrixMode (GL_MODELVIEW);
	glLoadMatrixf (matrix);
}


void
GLSpace::set_material (ShapeMaterial const& material)
{
	auto const& params = additional_parameters();
	auto emission_color = material.gl_emission_color;
	emission_color[3] *= params.alpha_factor;

	glFogCoordf (material.gl_fog_distance);
	glMaterialf (GL_FRONT, GL_SHININESS, 128.0 * material.gl_shininess);
	glColor4fv (emission_color);
	glMaterialfv (GL_FRONT, GL_EMISSION, emission_color);

	if (params.color_override)
	{
		auto color = *params.color_override;
		color[3] *= params.alpha_factor;
		glMaterialfv (GL_FRONT, GL_AMBIENT, *params.color_override);
		glMaterialfv (GL_FRONT, GL_DIFFUSE, *params.color_override);
		glMaterialfv (GL_FRONT, GL_SPECULAR, *params.color_override);
	}
	else if (params.alpha_factor != 1.0f)
	{
		auto ambient_color = material.gl_ambient_color;
		auto diffuse_color = material.gl_diffuse_color;
		auto specular_color = material.gl_specular_color;

		ambient_color[3] *= params.alpha_factor;
		diffuse_color[3] *= params.alpha_factor;
		specular_color[3] *= params.alpha_factor;

		glMaterialfv (GL_FRONT, GL_AMBIENT, ambient_color);
		glMaterialfv (GL_FRONT, GL_DIFFUSE, diffuse_color);
		glMaterialfv (GL_FRONT, GL_SPECULAR, specular_color);
	}
	else
	{
		glMaterialfv (GL_FRONT, GL_AMBIENT, material.gl_ambient_color);
		glMaterialfv (GL_FRONT, GL_DIFFUSE, material.gl_diffuse_color);
		glMaterialfv (GL_FRONT, GL_SPECULAR, material.gl_specular_color);
	}
}


void
GLSpace::set_texture (ShapeMaterial const& material)
{
	auto const& params = additional_parameters();
	auto color = material.gl_texture_color;
	color[3] *= params.alpha_factor;

	glFogCoordf (material.gl_fog_distance);
	glColor4fv (color);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, color);
	glTexCoord2f (material.texture_position.x(), material.texture_position.y());
}


void
GLSpace::draw (Shape const& shape)
{
	for (auto const& triangle: shape.triangles())
		begin (GL_TRIANGLES, triangle);

	for (auto const& strip: shape.triangle_strips())
		begin (GL_TRIANGLE_STRIP, strip);

	for (auto const& fan: shape.triangle_fans())
		begin (GL_TRIANGLE_FAN, fan);

	for (auto const& quad: shape.quads())
		begin (GL_QUADS, quad);
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

