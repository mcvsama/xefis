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

#ifndef XEFIS__SUPPORT__MATH__POSITION_ROTATION_H__INCLUDED
#define XEFIS__SUPPORT__MATH__POSITION_ROTATION_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/space.h>


namespace xf {

// TODO when CG is not at position, rotation of a body around CG must be preceeded by translation-to-CG before and translation-from-CG after the rotation
template<class pBaseFrame = void, class pBodyFrame = pBaseFrame>
	struct PositionRotation
	{
	  public:
		using BaseFrame			= pBaseFrame;
		using BodyFrame			= pBodyFrame;

		using Position			= SpaceVector<si::Length, BaseFrame>;
		using RotationToBase	= RotationMatrix<pBaseFrame, pBodyFrame>; // TODO remove 'p'
		using RotationToBody	= RotationMatrix<pBodyFrame, pBaseFrame>; // TODO remove 'p'

	  public:
		// Ctor
		PositionRotation() = default;

		// Ctor
		PositionRotation (Position const&, RotationToBody const&);

		// Ctor
		PositionRotation (Position const&, RotationToBase const&);

		/**
		 * Body position relative to the BaseFrame frame of reference.
		 */
		[[nodiscard]]
		Position const&
		position() const noexcept
			{ return _position; }

		/**
		 * Update body's position.
		 */
		void
		set_position (Position const& position)
			{ _position = position; }

		/**
		 * Body rotation matrix transforming from BaseFrame to BodyFrame.
		 */
		[[nodiscard]]
		RotationToBody const&
		base_to_body_rotation() const noexcept
			{ return _base_to_body_rotation; }

		/**
		 * Body rotation matrix transforming from BodyFrame to BaseFrame.
		 */
		[[nodiscard]]
		RotationToBase const&
		body_to_base_rotation() const noexcept
			{ return _body_to_base_rotation; }

		/**
		 * Set body's rotation matrix.
		 */
		void
		set_rotation (RotationToBody const&);

		/**
		 * Set body's rotation matrix.
		 */
		void
		set_rotation (RotationToBase const&);

		/**
		 * Translate the body by a relative vector in BaseFrame.
		 */
		void
		translate_frame (Position const& translation)
			{ _position += translation; }

		/**
		 * Translate the body by a relative vector in BodyFrame.
		 */
		void
		translate_frame (SpaceVector<si::Length, BodyFrame> const& vector)
			{ translate (_body_to_base_rotation * vector); }

		/**
		 * Rotate the body with the rotation vector about its position().
		 * Length represents rotation angle.
		 */
		void
		rotate_frame (SpaceVector<double, BaseFrame> const& rotation_vector)
			{ rotate_frame (make_pseudotensor (rotation_vector)); }

		/**
		 * Just like rotate_frame (...) but accepts rotation pseudotensor.
		 */
		void
		rotate_frame (RotationMatrix<BaseFrame> const& rotation_matrix);

		/**
		 * Rotate the body around different point than its position.
		 */
		void
		rotate_frame_about (Position const& about_point, SpaceVector<double, BaseFrame> const& rotation_vector)
			{ rotate_frame_about (about_point, make_pseudotensor (rotation_vector)); }

		/**
		 * Just like rotate_frame_about (...) but accepts rotation pseudotensor.
		 */
		void
		rotate_frame_about (Position const& about_point, RotationMatrix<BaseFrame> const& rotation_matrix);

		/**
		 * Transform vector from base frame of reference to body frame of referece.
		 */
		template<class Value>
			SpaceVector<Value, BodyFrame>
			transform_to_body (SpaceVector<Value, BaseFrame> const& vector) const
				{ return _base_to_body_rotation * (vector - _position); }

		/**
		 * Transform vector from body frame of reference to base frame of referece.
		 */
		template<class Value>
			SpaceVector<Value, BaseFrame>
			transform_to_base (SpaceVector<Value, BodyFrame> const& vector) const
				{ return _body_to_base_rotation * vector + _position; }

	  private:
		SpaceVector<si::Length, BaseFrame>		_position;
		RotationMatrix<BodyFrame, BaseFrame>	_base_to_body_rotation	{ math::unit };
		RotationMatrix<BaseFrame, BodyFrame>	_body_to_base_rotation	{ math::unit };
	};

} // namespace xf


// Local:
#include "position_rotation.tcc"

#endif

