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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__SHAPE_MATERIAL_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__SHAPE_MATERIAL_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/ui/gl_color.h>

// Qt:
#include <QColor>
#include <QOpenGLTexture>

// Standard:
#include <cstddef>
#include <memory>


namespace xf::rigid_body {

/**
 * Traits of the material of which ShapeVertices are made.
 */
struct ShapeMaterial
{
	GLColor				gl_emission_color	{ 0.0f, 0.0f, 0.0f, 1.0f };
	GLColor				gl_ambient_color	{ 1.0f, 1.0f, 1.0f, 1.0f };
	GLColor				gl_diffuse_color	{ 1.0f, 1.0f, 1.0f, 1.0f };
	GLColor				gl_specular_color	{ 1.0f, 1.0f, 1.0f, 1.0f };
	float				gl_shininess		{ 0.0f };
	float				gl_fog_distance		{ 0.0f };
	PlaneVector<float>	texture_position	{ math::zero };

	void
	set_emission_color (QColor const color)
		{ gl_emission_color = to_gl_color (color); }

	void
	set_ambient_color (QColor const color)
		{ gl_ambient_color = to_gl_color (color); }

	void
	set_diffuse_color (QColor const color)
		{ gl_diffuse_color = to_gl_color (color); }

	void
	set_specular_color (QColor const color)
		{ gl_specular_color = to_gl_color (color); }

	/**
	 * Set shininess in range [0..1].
	 */
	void
	set_shininess (float shininess)
		{ gl_shininess = shininess; }
};

} // namespace xf::rigid_body

#endif

