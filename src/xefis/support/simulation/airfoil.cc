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
				  SpaceVector<si::Length, BodyFrame> const& position,
				  si::Mass mass,
				  SpaceMatrix<si::MomentOfInertia, PartFrame> const& moment_of_inertia):
	BodyPart (position, mass, moment_of_inertia),
	_shape (shape)
{ }


ForceTorque<BodyFrame>
Airfoil::forces (Atmosphere::State<BodyFrame> const& atm_body)
{
	auto const body_to_airfoil_shape = z_rotation<AirfoilSplineFrame> (-_control.deflection_angle) * _body_to_airfoil_shape_transform;
	auto const airfoil_shape_to_body = inv (body_to_airfoil_shape);
	auto const atm_airfoil = Atmosphere::State<AirfoilSplineFrame> { atm_body.air, body_to_airfoil_shape * atm_body.wind };
	auto const planar_force_torque = _shape.planar_aerodynamic_forces (atm_airfoil, _control.angle_of_attack);
	Wrench const wrench {
		airfoil_shape_to_body * planar_force_torque.force(),
		airfoil_shape_to_body * planar_force_torque.torque(),
		position(),
	};

	return resultant_force (wrench);
}

} // namespace xf::sim

