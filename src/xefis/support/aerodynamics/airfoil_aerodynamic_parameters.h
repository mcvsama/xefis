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

#ifndef XEFIS__SUPPORT__AERODYNAMICS__AIRFOIL_AERODYNAMIC_PARAMETERS_H__INCLUDED
#define XEFIS__SUPPORT__AERODYNAMICS__AIRFOIL_AERODYNAMIC_PARAMETERS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/aerodynamics/airfoil_aerodynamic_forces.h>
#include <xefis/support/aerodynamics/angle_of_attack.h>
#include <xefis/support/aerodynamics/reynolds_number.h>
#include <xefis/support/atmosphere/air.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Air-data and forces computed for a given airfoil for given air conditions
 * and physical properties of the airfoil.
 */
template<class Space>
	class AirfoilAerodynamicParameters
	{
	  public:
		Air<Space>						air;
		ReynoldsNumber					reynolds_number;
		si::Velocity					true_air_speed;
		AngleOfAttack					angle_of_attack;
		AirfoilAerodynamicForces<Space>	forces;
	};

} // namespace xf

#endif

