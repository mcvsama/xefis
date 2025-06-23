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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>


namespace xf {

/*
 * NOTE
 * For rotation matrices this is true:
 * inv (R) == ~R
 */

template<math::CoordinateSystem pBaseSpace = void, math::CoordinateSystem pSpace = pBaseSpace>
	class Placement
	{
	  public:
		using BaseSpace				= pBaseSpace;
		using Space					= pSpace;

		using Position				= SpaceLength<BaseSpace>;
		using BodyRotation			= RotationQuaternion<BaseSpace, Space>;
		using BaseRotation			= RotationQuaternion<Space, BaseSpace>;
		using BodyRotationMatrix	= RotationMatrix<BaseSpace, Space>;
		using BaseRotationMatrix	= RotationMatrix<Space, BaseSpace>;

	  public:
		// Ctor
		Placement() = default;

		// Ctor
		Placement (Position const&, BaseRotation const&)
			requires (!std::is_same<BaseSpace, Space>());

		// Ctor
		Placement (Position const&, BodyRotation const&);

		/**
		 * Body position viewed from the BaseSpace coordinate system.
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
		 * Body rotation matrix transforming from BaseSpace to Space.
		 */
		[[nodiscard]]
		BaseRotation const&
		base_rotation() const noexcept
			{ return _base_rotation; }

		/**
		 * Return base's X, Y, Z axes viewed in BodySpace coordinates.
		 * Same as base_rotation().
		 */
		[[nodiscard]]
		BaseRotationMatrix
		base_coordinates() const noexcept
			{ return BaseRotationMatrix (_base_rotation); }

		/**
		 * Set body's rotation matrix.
		 */
		void
		set_base_rotation (BaseRotation const&);

		/**
		 * Body rotation matrix transforming from Space to BaseSpace.
		 */
		[[nodiscard]]
		BodyRotation const&
		body_rotation() const noexcept
			{ return _body_rotation; }

		/**
		 * Return body's X, Y, Z axes viewed in BaseSpace coordinates.
		 */
		[[nodiscard]]
		BodyRotationMatrix
		body_coordinates() const noexcept
			{ return BodyRotationMatrix (_body_rotation); }

		/**
		 * Set body's rotation matrix.
		 */
		void
		set_body_rotation (BodyRotation const&);

		/**
		 * Translate in-place the body by a relative vector in BaseSpace.
		 */
		void
		translate_frame (Position const& translation)
			{ _position += translation; }

		/**
		 * Translate in-place the body by a relative vector in Space.
		 */
		void
		translate_frame (SpaceLength<Space> const& vector)
			requires (!std::is_same<BaseSpace, Space>())
		{ translate (_body_rotation * vector); }

		/**
		 * Rotate in-place the body.
		 */
		void
		rotate_body_frame (RotationQuaternion<BaseSpace> const&);

		/**
		 * Rotate in-place the body around the 0 point in base frame of reference.
		 * Modifies both position vector and rotation matrix.
		 */
		void
		rotate_base_frame (RotationQuaternion<BaseSpace> const&);

		/**
		 * Rotate in-place the body around different point than origin.
		 * Point is represented in base frame of reference.
		 * Modifies both position vector and rotation matrix.
		 */
		void
		rotate_base_frame_about (Position const& about_point, RotationQuaternion<BaseSpace> const& rotation);

		/**
		 * Transform bound geometrical object from base to body space.
		 */
		template<class InputObject>
			[[nodiscard]]
			auto
			bound_transform_to_body (InputObject const& input) const
				{ return _base_rotation * (input - _position); }

		/**
		 * Transform unbound geometrical object from base to body space.
		 */
		template<class InputObject>
			[[nodiscard]]
			auto
			unbound_transform_to_body (InputObject const& input) const
				{ return _base_rotation * input; }

		/**
		 * Transform bound geometrical object from body to base space.
		 */
		template<class InputObject>
			[[nodiscard]]
			auto
			bound_transform_to_base (InputObject const& input) const
				{ return _body_rotation * input + _position; }

		/**
		 * Transform unbound geometrical object from body to base space.
		 */
		template<class InputObject>
			[[nodiscard]]
			auto
			unbound_transform_to_base (InputObject const& input) const
				{ return _body_rotation * input; }

	  private:
		SpaceLength<BaseSpace>	_position		{ math::zero };
		BaseRotation			_base_rotation	{ math::identity };
		BodyRotation			_body_rotation	{ math::identity };
	};


/*
 * Global functions
 */


/**
 * Reframe a placement into different spaces.
 */
template<math::CoordinateSystem NewBaseSpace, math::CoordinateSystem NewSpace, math::CoordinateSystem OldBaseSpace, math::CoordinateSystem OldSpace>
	[[nodiscard]]
	constexpr Placement<NewBaseSpace, NewSpace>&
	coordinate_system_cast (Placement<OldBaseSpace, OldSpace>& old)
	{
		return reinterpret_cast<Placement<NewBaseSpace, NewSpace>&> (old);
	}


/**
 * Reframe a placement into different spaces.
 */
template<math::CoordinateSystem NewBaseSpace, math::CoordinateSystem NewSpace, math::CoordinateSystem OldBaseSpace, math::CoordinateSystem OldSpace>
	[[nodiscard]]
	constexpr Placement<NewBaseSpace, NewSpace> const&
	coordinate_system_cast (Placement<OldBaseSpace, OldSpace> const& old)
	{
		return reinterpret_cast<Placement<NewBaseSpace, NewSpace> const&> (old);
	}


/*
 * Placement
 */


template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
	inline
	Placement<BaseSpace, Space>::Placement (Position const& position, BaseRotation const& rotation)
		requires (!std::is_same<BaseSpace, Space>()):
		_position (position),
		_base_rotation (rotation),
		_body_rotation (~rotation)
	{ }


template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
	inline
	Placement<BaseSpace, Space>::Placement (Position const& position, BodyRotation const& rotation):
		_position (position),
		_base_rotation (~rotation),
		_body_rotation (rotation)
	{ }


template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
	inline void
	Placement<BaseSpace, Space>::set_base_rotation (BaseRotation const& rotation)
	{
		_base_rotation = rotation;
		_body_rotation = ~rotation;
	}


template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
	inline void
	Placement<BaseSpace, Space>::set_body_rotation (BodyRotation const& rotation)
	{
		_body_rotation = rotation;
		_base_rotation = ~rotation;
	}


template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
	inline void
	Placement<BaseSpace, Space>::rotate_body_frame (RotationQuaternion<BaseSpace> const& rotation)
	{
		_body_rotation = rotation * _body_rotation;
		_base_rotation = ~_body_rotation;
	}


template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
	inline void
	Placement<BaseSpace, Space>::rotate_base_frame (RotationQuaternion<BaseSpace> const& rotation)
	{
		_position = rotation * _position;
		rotate_body_frame (rotation);
	}


template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
	inline void
	Placement<BaseSpace, Space>::rotate_base_frame_about (Position const& about_point, RotationQuaternion<BaseSpace> const& rotation)
	{
		_position -= about_point;
		rotate_base_frame (rotation);
		_position += about_point;
	}


template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space1, math::CoordinateSystem Space2>
	[[nodiscard]]
	inline auto
	relative_rotation (Placement<BaseSpace, Space1> const& from, Placement<BaseSpace, Space2> const& to)
	{
		// Divide the "from" rotation matrix by the "to" rotation matrix (mutiply by inversion):
		return from.base_rotation() * to.body_rotation();
	}


template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
	inline Placement<BaseSpace, Space>
	operator+ (Placement<BaseSpace, Space> placement, typename Placement<BaseSpace, Space>::Position const& vector)
	{
		placement.translate_frame (vector);
		return placement;
	}


template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
	inline Placement<BaseSpace, Space>
	operator- (Placement<BaseSpace, Space> placement, typename Placement<BaseSpace, Space>::Position const& vector)
	{
		placement.translate_frame (-vector);
		return placement;
	}

} // namespace xf

#endif

