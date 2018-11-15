/* vim:ts=4
 *
 * Copyleft 2012…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__MATH__POSITION_ROTATION_TCC__INCLUDED
#define XEFIS__SUPPORT__MATH__POSITION_ROTATION_TCC__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/space.h>
#include <xefis/support/math/geometry.h>


namespace xf {

template<class B, class O>
	inline
	PositionRotation<B, O>::PositionRotation (Position const& position, RotationToBody const& rotation):
		_position (position),
		_base_to_body_rotation (rotation),
		_body_to_base_rotation (inv (rotation))
	{ }


template<class B, class O>
	inline
	PositionRotation<B, O>::PositionRotation (Position const& position, RotationToBase const& rotation):
		_position (position),
		_base_to_body_rotation (inv (rotation)),
		_body_to_base_rotation (rotation)
	{ }


template<class B, class O>
	inline void
	PositionRotation<B, O>::set_rotation (RotationToBody const& rotation)
	{
		_base_to_body_rotation = rotation;
		_body_to_base_rotation = inv (rotation);
	}


template<class B, class O>
	inline void
	PositionRotation<B, O>::set_rotation (RotationToBase const& rotation)
	{
		_body_to_base_rotation = rotation;
		_base_to_body_rotation = inv (rotation);
	}


template<class B, class O>
	inline void
	PositionRotation<B, O>::rotate_frame (RotationMatrix<BaseFrame> const& rotation_matrix)
	{
		_body_to_base_rotation += rotation_matrix * _body_to_base_rotation;
		_body_to_base_rotation = vector_normalized (orthogonalized (_body_to_base_rotation));
		_base_to_body_rotation = inv (_body_to_base_rotation);
	}


template<class B, class O>
	inline void
	PositionRotation<B, O>::rotate_frame_about (Position const& about_point, RotationMatrix<BaseFrame> const& rotation_matrix)
	{
		// Rotate orientation:
		rotate (rotation_matrix);

		// Rotate position:
		_position -= about_point;
		_position = rotation_matrix * _position;
		_position += about_point;
	}

} // namespace xf

#endif

