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
#include <xefis/support/math/3d_space.h>


namespace xf::sim {

/**
 * Using ECEF (Earth-centered Earth-fixed) coordinates when describing things positioned on Earth.
 */
class Body
{
  public:
	/*
	 * Mass
	 */

	[[nodiscard]]
	si::Mass
	mass() const noexcept;

	void
	set_mass (si::Mass);

	/*
	 * Position (center of rest mass)
	 */

	[[nodiscard]]
	SpaceVector<si::Length> const&
	position() const noexcept;

	void
	set_position (SpaceVector<si::Length> const&);

	/*
	 * Velocity
	 */

	[[nodiscard]]
	SpaceVector<si::Velocity> const&
	velocity() const noexcept;

	void
	set_velocity (SpaceVector<si::Velocity> const&);

	/*
	 * Moment of inertia
	 */

	[[nodiscard]]
	SpaceMatrix<si::MomentOfInertia> const&
	moment_of_inertia() const noexcept;

	void
	set_moment_of_inertia (SpaceMatrix<si::MomentOfInertia> const&);

	/*
	 * Orientation
	 */

	[[nodiscard]]
	SpaceVector<double> const&
	orientation() const noexcept;

	void
	set_orientation (SpaceVector<double> const&);

	/*
	 * AngularVelocity
	 */

	[[nodiscard]]
	SpaceVector<si::BaseAngularVelocity> const&
	angular_velocity() const noexcept;

	void
	set_angular_velocity (SpaceVector<si::BaseAngularVelocity> const&);

	/**
	 * Update state of the object with given forces acting on it over the time dt.
	 */
	void
	evolve (SpaceVector<si::Force> const&, SpaceVector<si::Torque> const&, si::Time dt);

  public:
	// Basic physics:
	si::Mass											_mass;
	SpaceVector<si::Length>								_position;
	SpaceVector<si::Velocity>							_velocity;
	SpaceMatrix<si::MomentOfInertia>					_moment_of_inertia;
	SpaceMatrix<si::MomentOfInertia>::InversedMatrix	_inversed_moment_of_inertia;
	SpaceVector<double>									_orientation;
	SpaceVector<si::BaseAngularVelocity>				_angular_velocity;
};


inline si::Mass
Body::mass() const noexcept
{
	return _mass;
}


inline void
Body::set_mass (si::Mass mass)
{
	_mass = mass;
}


inline SpaceVector<si::Length> const&
Body::position() const noexcept
{
	return _position;
}


inline void
Body::set_position (SpaceVector<si::Length> const& position)
{
	_position = position;
}


inline SpaceVector<si::Velocity> const&
Body::velocity() const noexcept
{
	return _velocity;
}


inline void
Body::set_velocity (SpaceVector<si::Velocity> const& velocity)
{
	_velocity = velocity;
}


inline SpaceMatrix<si::MomentOfInertia> const&
Body::moment_of_inertia() const noexcept
{
	return _moment_of_inertia;
}


inline void
Body::set_moment_of_inertia (SpaceMatrix<si::MomentOfInertia> const& matrix)
{
	_moment_of_inertia = matrix;
	_inversed_moment_of_inertia = matrix.inversed();
}


inline SpaceVector<double> const&
Body::orientation() const noexcept
{
	return _orientation;
}


inline void
Body::set_orientation (SpaceVector<double> const& orientation)
{
	_orientation = orientation;
}


inline SpaceVector<si::BaseAngularVelocity> const&
Body::angular_velocity() const noexcept
{
	return _angular_velocity;
}


inline void
Body::set_angular_velocity (SpaceVector<si::BaseAngularVelocity> const& angular_velocity)
{
	_angular_velocity = angular_velocity;
}

} // namespace xf::sim

#endif

