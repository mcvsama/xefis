/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__AERODYNAMICS__AIRFOIL_AERODYNAMIC_FORCES_H__INCLUDED
#define XEFIS__SUPPORT__AERODYNAMICS__AIRFOIL_AERODYNAMIC_FORCES_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/air/air.h>
#include <xefis/support/nature/wrench.h>

// Standard:
#include <cstddef>


namespace xf {

template<class Space>
	class AirfoilAerodynamicForces
	{
	  public:
		SpaceForce<Space>	lift;
		SpaceForce<Space>	drag;
		SpaceTorque<Space>	pitching_moment;
		SpaceLength<Space>	center_of_pressure;

	  public:
		Wrench<Space>
		wrench() const
			{ return { ForceMoments<Space> (lift + drag, pitching_moment), center_of_pressure }; }
	};

} // namespace xf

#endif

