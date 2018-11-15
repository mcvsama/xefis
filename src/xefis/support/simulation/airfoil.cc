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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/simulation/airfoil_shape.h>

// Local:
#include "airfoil.h"


namespace xf::sim {

Airfoil::Airfoil (AirfoilShape const& shape,
				  PositionRotation<AirframeFrame, AirfoilFrame> const& position_rotation,
				  si::Mass mass,
				  SpaceMatrix<si::MomentOfInertia, AirfoilFrame> const& moment_of_inertia):
	BodyPart (position_rotation, mass, moment_of_inertia),
	_shape (shape)
{ }


ForceTorque<AirframeFrame>
Airfoil::forces (AtmosphereState<AirframeFrame> const& atm_body)
{
	auto const airfoil_shape_to_part_rotation = x_rotation<AirfoilFrame, void> (-90_deg) * y_rotation<void, AirfoilSplineFrame> (180_deg);
	auto const part_to_body_rotation = body_to_base_rotation() * airfoil_shape_to_part_rotation * z_rotation<AirfoilSplineFrame> (_control.deflection_angle);
	auto const body_to_part_rotation = inv (part_to_body_rotation);
	auto const atm_airfoil = AtmosphereState<AirfoilSplineFrame> { atm_body.air, body_to_part_rotation * atm_body.wind };
	auto const planar_force_torque = _shape.planar_aerodynamic_forces (atm_airfoil, _control.angle_of_attack);
	Wrench const wrench {
		part_to_body_rotation * planar_force_torque.force(),
		part_to_body_rotation * planar_force_torque.torque(),
		position(),
	};

	return resultant_force (wrench);
}

} // namespace xf::sim

