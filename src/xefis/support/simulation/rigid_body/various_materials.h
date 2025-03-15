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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__VARIOUS_MATERIALS_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__VARIOUS_MATERIALS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/shape_material.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

auto constexpr kBlackMatte = ShapeMaterial {
	.gl_emission_color	= GLColor { 0.0f, 0.0f, 0.0f, 1.0f },
	.gl_ambient_color	= GLColor { 0.0f, 0.0f, 0.0f, 1.0f },
	.gl_diffuse_color	= GLColor { 0.0f, 0.0f, 0.0f, 1.0f },
	.gl_specular_color	= GLColor { 0.0f, 0.0f, 0.0f, 1.0f },
	.gl_shininess		= 0.0f,
};

auto constexpr kWhiteMatte = ShapeMaterial {
	.gl_emission_color	= GLColor { 1.0f, 1.0f, 1.0f, 1.0f },
	.gl_ambient_color	= GLColor { 1.0f, 1.0f, 1.0f, 1.0f },
	.gl_diffuse_color	= GLColor { 1.0f, 1.0f, 1.0f, 1.0f },
	.gl_specular_color	= GLColor { 0.0f, 0.0f, 0.0f, 1.0f },
	.gl_shininess		= 0.0f,
};

auto constexpr kGreyMatte = ShapeMaterial {
	.gl_emission_color	= GLColor { 0.7f, 0.7f, 0.7f, 1.0f },
	.gl_ambient_color	= GLColor { 0.7f, 0.7f, 0.7f, 1.0f },
	.gl_diffuse_color	= GLColor { 0.7f, 0.7f, 0.7f, 1.0f },
	.gl_specular_color	= GLColor { 0.0f, 0.0f, 0.0f, 1.0f },
	.gl_shininess		= 0.0f,
};

auto constexpr kTransparentBlack = ShapeMaterial {
	.gl_emission_color	= GLColor { 0.0f, 0.0f, 0.0f, 0.0f },
	.gl_ambient_color	= GLColor { 0.0f, 0.0f, 0.0f, 0.0f },
	.gl_diffuse_color	= GLColor { 0.0f, 0.0f, 0.0f, 0.0f },
	.gl_specular_color	= GLColor { 0.0f, 0.0f, 0.0f, 0.0f },
	.gl_shininess		= 0.0f,
};


inline ShapeMaterial
make_material (QColor const& material_color, QColor const& emission_color = Qt::black)
{
	auto const gl_color = to_gl_color (material_color);
	return rigid_body::ShapeMaterial {
		.gl_emission_color	= to_gl_color (emission_color),
		.gl_ambient_color	= gl_color,
		.gl_diffuse_color	= gl_color,
		.gl_specular_color	= gl_color,
	};
}

} // namespace xf::rigid_body

#endif

