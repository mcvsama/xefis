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

// Standard:
#include <cstddef>
#include <initializer_list>


namespace xf::rigid_body {

/**
 * Traits of the material of which ShapeVertices are made.
 */
class ShapeMaterial
{
  public:
	/**
	 * Vertex emission color.
	 */
	[[nodiscard]]
	QColor const&
	emission_color() const noexcept
		{ return _emission_color; }

	/**
	 * Set vertex emission color.
	 */
	void
	set_emission_color (QColor const& color)
		{ _emission_color = color; }

	/**
	 * Vertex ambient color.
	 */
	[[nodiscard]]
	QColor const&
	ambient_color() const noexcept
		{ return _ambient_color; }

	/**
	 * Set vertex ambient color.
	 */
	void
	set_ambient_color (QColor const& color)
		{ _ambient_color = color; }

	/**
	 * Vertex diffuse color.
	 */
	[[nodiscard]]
	QColor const&
	diffuse_color() const noexcept
		{ return _diffuse_color; }

	/**
	 * Set vertex diffuse color.
	 */
	void
	set_diffuse_color (QColor const& color)
		{ _diffuse_color = color; }

	/**
	 * Vertex specular color.
	 */
	[[nodiscard]]
	QColor const&
	specular_color() const noexcept
		{ return _specular_color; }

	/**
	 * Set vertex specular color.
	 */
	void
	set_specular_color (QColor const& color)
		{ _specular_color = color; }

	/**
	 * Vertex shininess. Value range is 0..1.
	 */
	[[nodiscard]]
	float
	shininess() const noexcept
		{ return _shininess; }

	/**
	 * Set vertex shininess. Value range is 0..1.
	 */
	void
	set_shininess (float shininess) noexcept
		{ _shininess = shininess; }

	/**
	 * Return fog distance for the vertex.
	 */
	[[nodiscard]]
	float
	fog_distance() const noexcept
		{ return _fog_distance; }

	/**
	 * Set fog distance for the vertex.
	 */
	void
	set_fog_distance (float distance) noexcept
		{ _fog_distance = distance; }

  private:
	QColor	_emission_color	{ 0x00, 0x00, 0x00, 0xff };
	QColor	_ambient_color	{ 0xff, 0xff, 0xff, 0xff };
	QColor	_diffuse_color	{ 0xff, 0xff, 0xff, 0xff };
	QColor	_specular_color	{ 0xff, 0xff, 0xff, 0xff };
	float	_shininess		{ 0.1f };
	float	_fog_distance	{ 0.0f };
};

} // namespace xf::rigid_body

#endif

