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
#include <neutrino/numeric.h>
#include <neutrino/variant.h>

// Standard:
#include <cstddef>
#include <array>


namespace xf::rigid_body {

AngularServoConstraint::AngularServoConstraint (HingePrecalculation& hinge_precalculation, si::Angle backlash, ActionTorqueFunction action_torque_function, DragTorqueFunction drag_torque_function):
	Constraint (hinge_precalculation),
	Resistor ("AngularServoConstraint", kInitialResistance),
	_backlash (backlash),
	_failure_model (kExpectedLifetime, kNormalOperationTemperature, kAbsoluteMaximumTemperatureRange.max())
{
	_spring_constraint.emplace (hinge_precalculation, [this, action_torque_function, drag_torque_function] (si::Angle angle, SpaceVector<double, WorldSpace> const& hinge) -> si::Torque
	{
		auto const w1_about_hinge = projection_onto_normalized (Constraint::body_1().velocity_moments<WorldSpace>().angular_velocity(), hinge);
		auto const w2_about_hinge = projection_onto_normalized (Constraint::body_2().velocity_moments<WorldSpace>().angular_velocity(), hinge);
		auto const arm_velocity = w2_about_hinge - w1_about_hinge;

		_angular_velocity = (~arm_velocity * hinge).scalar();

		auto const error = angle - _setpoint;
		auto const action_scaler = std::clamp<si::Angle> (error, -_backlash, +_backlash) / _backlash;

		auto const action_torque = action_scaler * action_torque_function (error, voltage());
		auto const drag_torque = drag_torque_function (_angular_velocity, voltage());

		_torque = action_torque + drag_torque;

		return _torque;
	});
}


ConstraintForces
AngularServoConstraint::do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
											  VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
											  si::Time dt)
{
	return _spring_constraint->constraint_forces (vm_1, ext_forces_1, vm_2, ext_forces_2, dt);
}


void
AngularServoConstraint::calculated_constraint_forces (ConstraintForces const&)
{
	si::Power const mechanical_power = _torque * _angular_velocity / 1_rad;
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
			// If signs of torque and angular velocity are opposite, it means servo wants to add energy to the system. Prevent such case.
			if (sgn (_torque) == sgn (_angular_velocity))
				electrical_power = abs (_torque) / efficacy;
		},
	}, _efficiency_efficacy);

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
		set_broken (true);
		set_resistance (0.1_Ohm);
	}
}


std::unique_ptr<AngularServoConstraint>
make_standard_9gram_servo_constraint (HingePrecalculation& hinge_precalculation, float const scale)
{
	auto const action_torque_function = servo_action_torque (k9gramTorquePotential * scale);
	auto const drag_torque_function = servo_drag_torque (k9gramTorquePotential * scale, k9gramAngularVelocityPotential / scale);

	return std::make_unique<AngularServoConstraint> (hinge_precalculation, 1_deg, action_torque_function, drag_torque_function);
}

} // namespace xf::rigid_body

