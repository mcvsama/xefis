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

#ifndef XEFIS__SUPPORT__MATH__FRAME_OF_REFERENCE_H__INCLUDED
#define XEFIS__SUPPORT__MATH__FRAME_OF_REFERENCE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/space.h>
#include <xefis/support/math/position_rotation.h>


namespace xf {

// Tag used in one of FrameOfReference constructors:
inline struct RelativeToFrame { } relative_to_frame;


template<class pBaseFrame = void, class pBodyFrame = pBaseFrame>
	struct FrameOfReference: public PositionRotation<pBaseFrame, pBodyFrame>
	{
	  public:
		using BaseFrame			= pBaseFrame;
		using BodyFrame			= pBodyFrame;

		using PositionRotation	= xf::PositionRotation<BaseFrame, BodyFrame>;

		using Position			= typename PositionRotation::Position;
		using RotationToBase	= typename PositionRotation::RotationToBase;
		using RotationToBody	= typename PositionRotation::RotationToBody;

	  public:
		// Ctor
		explicit
		FrameOfReference() = default;

		// Ctor
		explicit
		FrameOfReference (RelativeToFrame, FrameOfReference const& base_frame);

		// Ctor
		explicit
		FrameOfReference (PositionRotation const&, FrameOfReference const& base_frame);

		// Ctor
		explicit
		FrameOfReference (Position const&, RotationToBody const&, FrameOfReference const& base_frame);

		// Ctor
		explicit
		FrameOfReference (Position const&, RotationToBase const&, FrameOfReference const& base_frame);

		// Copy ctor
		FrameOfReference (FrameOfReference const&) = default;

		// Move ctor
		FrameOfReference (FrameOfReference&&) = default;

		// Copy operator
		FrameOfReference&
		operator= (FrameOfReference const&) = default;

		// Move operator
		FrameOfReference&
		operator= (FrameOfReference&&) = default;

		/**
		 * Return pointer to base frame of reference.
		 */
		FrameOfReference const*
		base_frame() const noexcept
			{ return _base_frame; }

	  private:
		FrameOfReference const* _base_frame { nullptr };
	};


template<class B, class O>
	inline
	FrameOfReference<B, O>::FrameOfReference (RelativeToFrame, FrameOfReference const& base_frame):
		_base_frame (&base_frame)
	{ }


template<class B, class O>
	inline
	FrameOfReference<B, O>::FrameOfReference (PositionRotation const& position_rotation, FrameOfReference const& base_frame):
		PositionRotation (position_rotation),
		_base_frame (&base_frame)
	{ }


template<class B, class O>
	inline
	FrameOfReference<B, O>::FrameOfReference (Position const& position, RotationToBody const& rotation, FrameOfReference const& base_frame):
		PositionRotation (position, rotation),
		_base_frame (&base_frame)
	{ }


template<class B, class O>
	inline
	FrameOfReference<B, O>::FrameOfReference (Position const& position, RotationToBase const& rotation, FrameOfReference const& base_frame):
		PositionRotation (position, rotation),
		_base_frame (&base_frame)
	{ }


/*
 * Global functions
 */


// TODO
/**
 * Transform by another frame of reference.
 */
#if 0
inline FrameOfReference
operator* (FrameOfReference const& a, FrameOfReference const& b)
{
	Resulting frame will have .base = b.base;
	Also do translation difference.
}
#endif

} // namespace xf

#endif

