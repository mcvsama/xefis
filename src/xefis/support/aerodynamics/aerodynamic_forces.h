/* vim:ts=4
 *
 * Copyleft 2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__AERODYNAMICS__AERODYNAMIC_FORCES_H__INCLUDED
#define XEFIS__SUPPORT__AERODYNAMICS__AERODYNAMIC_FORCES_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/atmosphere/air.h>
#include <xefis/support/nature/wrench.h>

// Standard:
#include <cstddef>


namespace xf {

template<class Space>
	class AerodynamicForces
	{
	  public:
		SpaceForce<Space>	lift;
		SpaceForce<Space>	induced_drag;
		SpaceForce<Space>	parasitic_drag;
		SpaceTorque<Space>	pitching_moment;
		SpaceLength<Space>	center_of_pressure;

	  public:
		[[nodiscard]]
		SpaceForce<Space>
		total_drag() const noexcept
			{ return induced_drag + parasitic_drag; }

		Wrench<Space>
		wrench() const
			{ return { ForceMoments<Space> (lift + total_drag(), pitching_moment), center_of_pressure }; }
	};

} // namespace xf

#endif
