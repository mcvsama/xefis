/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__BODY_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__BODY_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <lib/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/space.h>
#include <xefis/support/nature/physics.h>
#include <xefis/support/simulation/body_shape.h>


namespace xf::sim {

/**
 * Using ECEF (Earth-centered Earth-fixed) coordinates when describing things positioned on Earth.
 */
class Body
{
  public:
	// Ctor
	explicit
	Body (BodyShape&&);

	/*
	 * Shape of the body.
	 */
	[[nodiscard]]
	BodyShape&
	shape() noexcept
		{ return _shape; }

	/*
	 * Shape of the body.
	 */
	[[nodiscard]]
	BodyShape const&
	shape() const noexcept
		{ return _shape; }

	/**
	 * Set new shape of the body.
	 */
	void
	set_shape (BodyShape&& shape)
		{ _shape = std::move (shape); }

	/*
	 * Position (center of rest mass)
	 */

	[[nodiscard]]
	SpaceVector<si::Length, ECEFFrame> const&
	position() const noexcept
		{ return _position; }

	void
	set_position (SpaceVector<si::Length, ECEFFrame> const& position)
		{ _position = position; }

	/**
	 * Velocity.
	 */
	[[nodiscard]]
	SpaceVector<si::Velocity, ECEFFrame> const&
	velocity() const noexcept
		{ return _velocity; }

	/**
	 * Set new velocity.
	 */
	void
	set_velocity (SpaceVector<si::Velocity, ECEFFrame> const& velocity)
		{ _velocity = velocity; }

	/**
	 * Angular velocity.
	 */
	[[nodiscard]]
	SpaceVector<si::BaseAngularVelocity, ECEFFrame> const&
	angular_velocity() const noexcept
		{ return _angular_velocity; }

	/**
	 * Set new angular velocity.
	 */
	void
	set_angular_velocity (SpaceVector<si::BaseAngularVelocity, ECEFFrame> const& angular_velocity)
		{ _angular_velocity = angular_velocity; }

	/**
	 * Orientation tensor.
	 * Since this is in ECEF frame, which is just basis vectors of matrix,
	 * orientation tensor is the same as rotation matrix for this object.
	 */
	[[nodiscard]]
	SpaceMatrix<double, ECEFFrame, BodyFrame> const&
	orientation() const noexcept
		{ return _body_to_ecef_transform; }

	/**
	 * Set new orientation tensor.
	 */
	void
	set_orientation (SpaceMatrix<double, ECEFFrame, BodyFrame> const&);

	/**
	 * Return body-to-ecef transformation matrix.
	 */
	SpaceMatrix<double, ECEFFrame, BodyFrame> const&
	body_to_ecef_transform() const noexcept
		{ return _body_to_ecef_transform; }

	/**
	 * Return ecef-to-body transformation matrix.
	 */
	SpaceMatrix<double, BodyFrame, ECEFFrame> const&
	ecef_to_body_transform() const noexcept
		{ return _ecef_to_body_transform; }

	/**
	 * Act on the body with given forces over time dt.
	 */
	void
	act (ForceTorque<ECEFFrame> const& force_torque, si::Time dt);

  private:
	BodyShape										_shape;
	// Basic physics:
	SpaceVector<si::Length, ECEFFrame>				_position;
	SpaceVector<si::Velocity, ECEFFrame>			_velocity;
	SpaceVector<si::BaseAngularVelocity, ECEFFrame>	_angular_velocity;
	// Orientation transforms:
	SpaceMatrix<double, ECEFFrame, BodyFrame>		_body_to_ecef_transform;
	SpaceMatrix<double, BodyFrame, ECEFFrame>		_ecef_to_body_transform;
};


inline void
Body::set_orientation (SpaceMatrix<double, ECEFFrame, BodyFrame> const& orientation)
{
	_body_to_ecef_transform = vector_normalized (orthogonalized (orientation));
	_ecef_to_body_transform = inv (_body_to_ecef_transform);
}

} // namespace xf::sim

#endif

