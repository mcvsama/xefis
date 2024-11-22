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
#include "angular_servo_constraint.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Neutrino:
#include <neutrino/math/matrix_operations.h>
#include <neutrino/numeric.h>
#include <neutrino/variant.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

AngularServoConstraint::AngularServoConstraint (HingePrecalculation& hinge_precalculation,
												Range<si::Angle> const angle_range,
												si::Angle const backlash,
												AngularVelocityPotential const angular_velocity_potential,
												TorquePotential const torque_potential):
	Constraint (hinge_precalculation),
	Resistor ("AngularServoConstraint", kInitialResistance),
	_hinge (hinge_precalculation),
	_pid_controller ({ .p = 50.0, .i = 1.0, .d = 1.0 }, 0_deg),
	_angle_range (angle_range),
	_backlash (backlash),
	_failure_model (kExpectedLifetime, kNormalOperationTemperature, kAbsoluteMaximumTemperatureRange.max()),
	_angular_velocity_potential (angular_velocity_potential),
	_torque_potential (torque_potential),
	_motor_constraint (hinge_precalculation)
{
	set_label ("angular servo");
	_pid_controller.set_integral_limit ({ -0.1_deg * 1_s, +0.1_deg * 1_s });
	_pid_controller.set_output_limit ({ -1.0, +1.0 });
}


void
AngularServoConstraint::initialize_step (si::Time const dt)
{
	_motor_constraint.initialize_step (dt);
}


ConstraintForces
AngularServoConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time const dt)
{
	return _motor_constraint.constraint_forces (vm_1, vm_2, dt);
}


void
AngularServoConstraint::calculated_constraint_forces (ConstraintForces const& result, si::Time const dt)
{
	Constraint::calculated_constraint_forces (result, dt);

	update_velocity_and_torque();
	update_pid_controller (dt);

	si::Power const mechanical_power = _arm_torque * _arm_angular_velocity / 1_rad;
	si::Power electrical_power = 0_W;

	std::visit (overload {
		[&] (std::monostate) noexcept {
			// Assume ideal servo.
			electrical_power = mechanical_power;
		},
		[&] (double efficiency) noexcept {
			// Because of the non-zero backlash it's possible to get torque and angular velocity having opposite signs thus giving negative values of power
			// here. But real servo doesn't really add power to the circuit, so make sure that power is clamped here is at least 0 W.
			electrical_power = std::max (0_W, mechanical_power) / efficiency;
		},
		[&] (TorqueEfficacy efficacy) noexcept {
			// If signs of torque and angular velocity are opposite, it means servo wants to add energy to the system. Prevent such case, because we don't
			// want trouble in our crude electrical simulation:
			if (sgn (_arm_torque) == sgn (_arm_angular_velocity))
				electrical_power = abs (_arm_torque) / efficacy;
		},
	}, _effic);

	si::Current const current = electrical_power / voltage();
	set_resistance (voltage() / current);
	_power_loss = electrical_power - mechanical_power;
}


void
AngularServoConstraint::flow_current (si::Time const dt)
{
	if (!kAbsoluteMaximumVoltageRange.includes (voltage()) ||
		!kAbsoluteMaximumTemperatureRange.includes (temperature()) ||
		_failure_model.should_fail (temperature(), dt))
	{
		Element::set_broken (true);
		set_resistance (0.1_Ohm);
	}
}


void
AngularServoConstraint::update_velocity_and_torque()
{
	auto const pl_1 = Constraint::body_1().placement();
	auto const hinge = pl_1.unbound_transform_to_base (_hinge.body_1_normalized_hinge()) / abs (_hinge.body_1_normalized_hinge());

	auto const w1_about_hinge = projection_onto_normalized (Constraint::body_1().velocity_moments<WorldSpace>().angular_velocity(), hinge);
	auto const w2_about_hinge = projection_onto_normalized (Constraint::body_2().velocity_moments<WorldSpace>().angular_velocity(), hinge);
	auto const arm_velocity = w2_about_hinge - w1_about_hinge;
	_arm_angular_velocity = dot_product (arm_velocity, hinge);

	auto const t1_about_hinge = projection_onto_normalized (Constraint::body_1().iteration().all_constraints_force_moments.torque(), hinge);
	auto const t2_about_hinge = projection_onto_normalized (Constraint::body_2().iteration().all_constraints_force_moments.torque(), hinge);
	auto const arm_torque = t2_about_hinge - t1_about_hinge;
	_arm_torque = dot_product (arm_torque, hinge);
}


void
AngularServoConstraint::update_pid_controller (si::Time const dt)
{
	auto const error = _hinge.data().angle - _setpoint;

	if (abs (error) < _backlash)
	{
		_motor_constraint.set_abs_torque (0_Nm);
		_motor_constraint.set_max_angular_velocity (0_radps);
	}
	else
	{
		auto const velocity_factor = _pid_controller.process (_setpoint, _hinge.data().angle, dt);
		auto const angular_velocity = _angular_velocity_potential * voltage();

		_motor_constraint.set_abs_torque (_torque_potential * voltage());
		_motor_constraint.set_max_angular_velocity (velocity_factor * angular_velocity);
	}
}


std::unique_ptr<AngularServoConstraint>
make_standard_servo_constraint (HingePrecalculation& hinge_precalculation, float scale)
{
	auto const angular_velocity_potential = kStandardAngularVelocityPotential / std::pow (scale, 0.25); // More or less, not being precise here.
	auto const torque_potential = kStandardTorquePotential * scale;
	return std::make_unique<AngularServoConstraint> (hinge_precalculation, Range { -90_deg, +90_deg }, 0.5_deg, angular_velocity_potential, torque_potential);
}


std::unique_ptr<AngularServoConstraint>
make_standard_9gram_servo_constraint (HingePrecalculation& hinge_precalculation, float scale)
{
	auto const angular_velocity_potential = k9gramAngularVelocityPotential / std::pow (scale, 0.25); // More or less, not being precise here.
	auto const torque_potential = k9gramTorquePotential * scale;
	return std::make_unique<AngularServoConstraint> (hinge_precalculation, Range { -90_deg, +90_deg }, 0.5_deg, angular_velocity_potential, torque_potential);
}

} // namespace xf::rigid_body

