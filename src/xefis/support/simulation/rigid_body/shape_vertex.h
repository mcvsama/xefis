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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__SHAPE_VERTEX_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__SHAPE_VERTEX_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/simulation/rigid_body/shape_material.h>

// Standard:
#include <cstddef>
#include <initializer_list>


namespace xf::rigid_body {

/**
 * Vertex of the shape used to define usually triangular surfaces.
 */
class ShapeVertex
{
  public:
	// Ctor
	ShapeVertex (std::initializer_list<si::Length> coordinates);

	// Ctor
	explicit
	ShapeVertex (SpaceLength<BodyOrigin> const& position);

	// Ctor
	explicit
	ShapeVertex (SpaceLength<BodyOrigin> const& position, ShapeMaterial const&);

	/**
	 * \param	normal
	 *			Vector normal to the surface at the vertex position.
	 */
	explicit
	ShapeVertex (SpaceLength<BodyOrigin> const& position, SpaceVector<double, BodyOrigin> const& normal);

	/**
	 * \param	normal
	 *			Vector normal to the surface at the vertex position.
	 */
	explicit
	ShapeVertex (SpaceLength<BodyOrigin> const& position, SpaceVector<double, BodyOrigin> const& normal, ShapeMaterial const&);

	/**
	 * Return vertex position in space.
	 */
	[[nodiscard]]
	SpaceLength<BodyOrigin> const&
	position() const noexcept
		{ return _position; }

	/**
	 * Set new vertex position.
	 */
	void
	set_position (SpaceLength<BodyOrigin> const& position)
		{ _position = position; }

	/**
	 * Return normalized normal vector (if exists).
	 */
	[[nodiscard]]
	std::optional<SpaceVector<double, BodyOrigin>> const&
	normal() const noexcept
		{ return _normal; }

	/**
	 * Set new vertex normal.
	 */
	void
	set_normal (std::optional<SpaceVector<double, BodyOrigin>> const& normal)
		{ _normal = normal; }

	/**
	 * Shape material.
	 */
	[[nodiscard]]
	ShapeMaterial const&
	material() const noexcept
		{ return _material; }

	/**
	 * Set shape material.
	 */
	void
	set_material (ShapeMaterial const& material)
		{ _material = material; }

	/**
	 * Transform the vertex.
	 */
	void
	transform (AffineTransform<BodyOrigin> const&);

	/**
	 * Rotate the vertex about space origin by provided rotation matrix.
	 */
	void
	rotate (RotationQuaternion<BodyOrigin> const&);

	/**
	 * Translate the vertex by given vector.
	 */
	void
	translate (SpaceLength<BodyOrigin> const& translation)
        { _position += translation; }

  private:
	SpaceLength<BodyOrigin>							_position;
	std::optional<SpaceVector<double, BodyOrigin>>	_normal;
	ShapeMaterial									_material;
};


inline
ShapeVertex::ShapeVertex (std::initializer_list<si::Length> coordinates):
	_position (coordinates.begin(), coordinates.end())
{ }


inline
ShapeVertex::ShapeVertex (SpaceLength<BodyOrigin> const& position):
	_position (position)
{ }


inline
ShapeVertex::ShapeVertex (SpaceLength<BodyOrigin> const& position, ShapeMaterial const& material):
	_position (position),
	_material (material)
{ }


inline
ShapeVertex::ShapeVertex (SpaceLength<BodyOrigin> const& position, SpaceVector<double, BodyOrigin> const& normal):
	_position (position),
	_normal (normal)
{ }


inline
ShapeVertex::ShapeVertex (SpaceLength<BodyOrigin> const& position, SpaceVector<double, BodyOrigin> const& normal, ShapeMaterial const& material):
	_position (position),
	_normal (normal),
	_material (material)
{ }

} // namespace xf::rigid_body

#endif

