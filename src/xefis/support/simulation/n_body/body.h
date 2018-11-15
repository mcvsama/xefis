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

#ifndef XEFIS__SUPPORT__SIMULATION__N_BODY__BODY_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__N_BODY__BODY_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <lib/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/space.h>
#include <xefis/support/nature/physics.h>
#include <xefis/support/simulation/n_body/body_shape.h>


namespace xf::sim {

/**
 * Using ECEF (Earth-centered Earth-fixed) coordinates when describing things positioned on Earth.
 */
class Body: public PositionRotation<ECEFFrame, AirframeFrame>
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
	 * Act on the body with given forces over time dt.
	 */
	void
	act (ForceTorque<ECEFFrame> const& force_torque, si::Time dt);

  private:
	BodyShape										_shape;
	SpaceVector<si::Velocity, ECEFFrame>			_velocity;
	SpaceVector<si::BaseAngularVelocity, ECEFFrame>	_angular_velocity;
};

} // namespace xf::sim

#endif

