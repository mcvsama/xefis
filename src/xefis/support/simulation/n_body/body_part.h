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

#ifndef XEFIS__SUPPORT__SIMULATION__N_BODY__BODY_PART_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__N_BODY__BODY_PART_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/physics.h>
#include <xefis/support/math/position_rotation.h>
#include <xefis/support/math/space.h>
#include <xefis/support/simulation/atmosphere.h>


namespace xf::sim {

class BasicBodyPart
{
  public:
	// Dtor
	virtual ~BasicBodyPart() = default;

	/**
	 * Calculate forces acting on the part.
	 */
	[[nodiscard]]
	virtual ForceTorque<AirframeFrame>
	forces (AtmosphereState<AirframeFrame> const&)
		{ return { math::zero, math::zero }; }

	// TODO co z tym? Może const&
	[[nodiscard]]
	virtual SpaceVector<si::Length, AirframeFrame>
	aircraft_relative_position() const noexcept = 0;

	[[nodiscard]]
	virtual si::Mass
	mass() const noexcept = 0;
};


template<class pAirframeFrame, class pPartFrame>
	class BodyPart:
		public BasicBodyPart,
		public PositionRotation<pAirframeFrame, pPartFrame>
	{
	  public:
		using AirframeFrame	= pAirframeFrame;
		using PartFrame		= pPartFrame;

	  public:
		// Ctor
		explicit
		BodyPart (PositionRotation<AirframeFrame, PartFrame> const& position_rotation,
				  si::Mass,
				  SpaceMatrix<si::MomentOfInertia, PartFrame> const& moment_of_inertia);

		/**
		 * Rest mass.
		 */
		[[nodiscard]]
		si::Mass
		mass() const noexcept override
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

		// BasicBodyPart API
		SpaceVector<si::Length, AirframeFrame>
		aircraft_relative_position() const noexcept
			{ return this->position(); }

	  private:
		si::Mass																_mass;
		SpaceMatrix<si::MomentOfInertia, PartFrame>								_moment_of_inertia			{ math::zero };
		typename SpaceMatrix<si::MomentOfInertia, PartFrame>::InversedMatrix	_inversed_moment_of_inertia	{ math::zero };
		// TODO implement
		//// Position measured from the CG of the body in body frame of reference; calculated and cached here:
		//SpaceVector<si::Length, BodyFrame>				_computed_com_position		{ math::zero };
		//// Computed moment of inertia when measured from _computed_com_position:
		//SpaceMatrix<si::MomentOfInertia, PartFrame>		_computed_moment_of_inertia	{ math::zero };
	};


template<class AF, class PF>
	inline
	BodyPart<AF, PF>::BodyPart (PositionRotation<AirframeFrame, PartFrame> const& position_rotation,
								si::Mass mass,
								SpaceMatrix<si::MomentOfInertia, PartFrame> const& moment_of_inertia):
		PositionRotation<AirframeFrame, PartFrame> (position_rotation),
		_mass (mass)
	{
		set_moment_of_inertia (moment_of_inertia);
	}


template<class AF, class PF>
	inline void
	BodyPart<AF, PF>::set_moment_of_inertia (SpaceMatrix<si::MomentOfInertia, PartFrame> const& moment_of_inertia)
	{
		_moment_of_inertia = moment_of_inertia;
		_inversed_moment_of_inertia = inv (moment_of_inertia);
	}

} // namespace xf::sim

#endif

