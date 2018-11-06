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

#ifndef XEFIS__SUPPORT__SIMULATION__AIRFRAME_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__AIRFRAME_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/air.h>
#include <xefis/support/simulation/atmosphere.h>
#include <xefis/support/simulation/body.h>


namespace xf::sim {

// Perhaps it's possible to get rid of Airframe and move forces() and stuff into Body alone?
class Airframe: public Body
{
  public:
	/**
	 * Ctor
	 *
	 * \param	BodyShape
	 *			Shape of the airframe. Get copied inside the Aiframe object.
	 */
	explicit
	Airframe (BodyShape&&);

	/**
	 * Calculate total forces acting on a body, excluding gravity.
	 * Forces are expressed in ECEF frame of reference.
	 */
	[[nodiscard]]
	ForceTorque<ECEFFrame>
	forces (Atmosphere const& atmosphere) const;

	/**
	 * Return atmosphere state in body frame for given position relative to center-of-mass of the body.
	 * Takes into account things like relative wind due to body rotation at the part position.
	 * Note the wind will be a relative wind to the airframe.
	 */
	[[nodiscard]]
	Atmosphere::State<BodyFrame>
	complete_atmosphere_state_at (SpaceVector<si::Length, BodyFrame> com_relative_part_position, Atmosphere const& atmosphere) const;
};

} // namespace xf::sim

#endif

