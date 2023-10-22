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
	.emission_color	= QColor { 0, 0, 0, 0 },
	.ambient_color	= QColor { 0, 0, 0, 0 },
	.diffuse_color	= QColor { 0, 0, 0, 0 },
	.specular_color	= QColor { 0, 0, 0, 0 },
	.shininess		= 0.0,
};

auto constexpr kWhiteMatte = ShapeMaterial {
	.emission_color	= QColor { 0xff, 0xff, 0xff, 0xff },
	.ambient_color	= QColor { 0xff, 0xff, 0xff, 0xff },
	.diffuse_color	= QColor { 0xff, 0xff, 0xff, 0xff },
	.specular_color	= QColor { 0xff, 0xff, 0xff, 0xff },
	.shininess		= 0.0,
};

auto constexpr kGreyMatte = ShapeMaterial {
	.emission_color	= QColor { 180, 180, 180, 0xff },
	.ambient_color	= QColor { 180, 180, 180, 0xff },
	.diffuse_color	= QColor { 180, 180, 180, 0xff },
	.specular_color	= QColor { 180, 180, 180, 0xff },
	.shininess		= 0.0,
};


inline ShapeMaterial
make_material (QColor const& color)
{
	return rigid_body::ShapeMaterial {
		.ambient_color	= color,
		.diffuse_color	= color,
		.specular_color	= color,
	};
}

} // namespace xf::rigid_body

#endif

