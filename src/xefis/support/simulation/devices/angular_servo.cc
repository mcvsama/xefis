/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Local:
#include "angular_servo.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/nature/various_inertia_tensors.h>
#include <xefis/support/simulation/constraints/angular_servo_constraint.h>
#include <xefis/support/simulation/rigid_body/various_shapes.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <array>


namespace xf::sim {

AngularServo::AngularServo (rigid_body::AngularServoConstraint& constraint, si::Angle const resolution, MassMomentsAtArm<BodyCOM> const& mass_moments):
	Body (mass_moments),
	_constraint (constraint),
	_resolution (resolution)
{
	set_shape (xf::rigid_body::make_centered_cube_shape (mass_moments));
}


void
AngularServo::set_setpoint (si::Angle const setpoint)
{
	_constraint.set_setpoint (neutrino::quantized (setpoint, 1 / _resolution, _constraint.angle_range()));
}


std::unique_ptr<AngularServo>
make_standard_servo (rigid_body::AngularServoConstraint& constraint, float scale)
{
	auto const mass = 40_gr * scale;
	auto const mass_moments = MassMoments<BodyCOM> (mass, make_cuboid_inertia_tensor<BodyCOM> (mass, { 40_mm * scale, 20_mm * scale, 36_mm * scale }));
	return std::make_unique<AngularServo> (constraint, 0.5_deg, mass_moments);
}


std::unique_ptr<AngularServo>
make_standard_9gram_servo (rigid_body::AngularServoConstraint& constraint)
{
	auto const mass = 9_gr;
	auto const mass_moments = MassMoments<BodyCOM> (mass, make_cuboid_inertia_tensor<BodyCOM> (mass, { 24_mm, 12_mm, 28_mm }));
	return std::make_unique<AngularServo> (constraint, 0.5_deg, mass_moments);
}

} // namespace xf::sim

