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

// Qt:
#include <QColor>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

/**
 * Traits of the material of which ShapeVertices are made.
 */
struct ShapeMaterial
{
	QColor	emission_color	{ 0x00, 0x00, 0x00, 0xff };
	QColor	ambient_color	{ 0xff, 0xff, 0xff, 0xff };
	QColor	diffuse_color	{ 0xff, 0xff, 0xff, 0xff };
	QColor	specular_color	{ 0xff, 0xff, 0xff, 0xff };
	float	shininess		{ 0.1f };
	float	fog_distance	{ 0.0f };
};

} // namespace xf::rigid_body

#endif

