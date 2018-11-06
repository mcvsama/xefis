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

#ifndef XEFIS__SUPPORT__SIMULATION__BODY_PART_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__BODY_PART_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/physics.h>
#include <xefis/support/simulation/atmosphere.h>


namespace xf::sim {

class BodyPart
{
  public:
	// Ctor
	explicit
	BodyPart (SpaceVector<si::Length, BodyFrame> const& position,
			  si::Mass,
			  SpaceMatrix<si::MomentOfInertia, PartFrame> const& moment_of_inertia);

	// Dtor
	virtual ~BodyPart() = default;

	/**
	 * Position of the part at which resultant forces act.
	 */
	[[nodiscard]]
	SpaceVector<si::Length, BodyFrame> const&
	position() const noexcept
		{ return _position; }

	/**
	 * Set new position of the part.
	 */
	void
	set_position (SpaceVector<si::Length, BodyFrame> const& position)
		{ _position = position; }

	/**
	 * Rest mass.
	 */
	[[nodiscard]]
	si::Mass
	mass() const noexcept
		{ return _mass; }

	/**
	 * Set new rest mass.
	 */
	void
	set_mass (si::Mass mass)
		{ _mass = mass; }

	/**
	 * Moment of inertia tensor about the center of mass.
	 */
	[[nodiscard]]
	SpaceMatrix<si::MomentOfInertia, PartFrame> const&
	moment_of_inertia() const noexcept
		{ return _moment_of_inertia; }

	/**
	 * Set new moment of inertia tensor.
	 */
	void
	set_moment_of_inertia (SpaceMatrix<si::MomentOfInertia, PartFrame> const&);

	/**
	 * Calculate forces acting on the part.
	 */
	[[nodiscard]]
	virtual ForceTorque<BodyFrame>
	forces (Atmosphere::State<BodyFrame> const&) = 0;

  private:
	// Position measured from arbitrary user-defined point of reference:
	SpaceVector<si::Length, BodyFrame>							_position					{ math::zero };
	si::Mass													_mass;
	SpaceMatrix<si::MomentOfInertia, PartFrame>					_moment_of_inertia			{ math::zero };
	SpaceMatrix<si::MomentOfInertia, PartFrame>::InversedMatrix	_inversed_moment_of_inertia	{ math::zero };
	// TODO implement
	//// Position measured from the CG of the body in body frame of reference; calculated and cached here:
	//SpaceVector<si::Length, BodyFrame>				_computed_com_position		{ math::zero };
	//// Computed moment of inertia when measured from _computed_com_position:
	//SpaceMatrix<si::MomentOfInertia, PartFrame>		_computed_moment_of_inertia	{ math::zero };
};

} // namespace xf::sim

#endif

