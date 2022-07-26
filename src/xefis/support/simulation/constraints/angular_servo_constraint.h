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

#ifndef XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__ANGULAR_SERVO_CONSTRAINT_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__CONSTRAINTS__ANGULAR_SERVO_CONSTRAINT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/components/resistor.h>
#include <xefis/support/simulation/constraints/angular_spring_constraint.h>
#include <xefis/support/simulation/constraints/hinge_precalculation.h>
#include <xefis/support/simulation/failure/sigmoidal_temperature_failure.h>
#include <xefis/support/simulation/rigid_body/body.h>

// Neutrino:
#include <neutrino/range.h>

// Standard:
#include <cstddef>
#include <memory>
#include <optional>
#include <variant>


namespace xf::rigid_body {

// Typical parameters describing RC model servos:
using AngularVelocityPotential	= decltype (1_rad / 1_s / 1_V);
using TorquePotential			= decltype (1_Nm / 1_V);

// Typical microservo angular velocity potential: 83°/s/V (about 500°/s at 6 V):
static constexpr auto k9gramAngularVelocityPotential	= 83_deg / 1_s / 1_V;

// Typical microservo torque potential: ~0.024 Nm/V (0.144 Nm at 6 V):
static constexpr auto k9gramTorquePotential				= 0.024_Nm / 1_V;


/**
 * Simplfied angular servomechanism constraint and electric device to use in
 * electrical and n-body simulators. Acts with a torque like a servo to make the arm
 * move to a specified set point angle.
 */
class AngularServoConstraint:
	public Constraint,
	public electrical::Resistor
{
  public:
	using ActionTorqueFunction	= std::function<si::Torque (si::Angle error, si::Voltage)>;
	using DragTorqueFunction	= std::function<si::Torque (si::AngularVelocity const& arm_velocity, si::Voltage)>;
	using TorqueEfficacy		= decltype (1_Nm / 1_W);

	static constexpr si::Resistance			kInitialResistance					{ 1_kOhm };
	static constexpr si::Time				kExpectedLifetime					{ 1000 * 365 * 86400_s };
	static constexpr si::Temperature		kNormalOperationTemperature			{ 25_degC };
	// Absolute maximum ratings:
	static constexpr Range<si::Voltage>		kAbsoluteMaximumVoltageRange		{ -0.1_V, 7.2_V };
	static constexpr Range<si::Temperature>	kAbsoluteMaximumTemperatureRange	{ -10_degC, 70_degC };

  public:
	/**
	 * Ctor
	 * \param	backlash
	 *			If error (actual - setpoint) is greater or equal to backlash,
	 *			full available force will be used. Otherwise it will linearly depend on the error.
	 */
	explicit
	AngularServoConstraint (HingePrecalculation&, si::Angle backlash, ActionTorqueFunction, DragTorqueFunction);

	/**
	 * Set servo setpoint.
	 */
	void
	set_setpoint (si::Angle setpoint)
		{ _setpoint = setpoint; }

	/**
	 * Set electrical efficiency.
	 * Setting this invalidates value set by set_efficacy().
	 *
	 * \param	efficiency_factor
	 *			Should be in range 0…1.
	 */
	void
	set_efficiency (double efficiency_factor)
		{ _efficiency_efficacy = efficiency_factor; }

	/**
	 * Set electrical efficacy.
	 * Setting this invalidates value set by set_efficiency().
	 *
	 * \param	efficacy
	 *			Torque per power (Nm per Watt).
	 */
	void
	set_efficacy (TorqueEfficacy efficacy)
		{ _efficiency_efficacy = efficacy; }

	// Constraint API
	ConstraintForces
	do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, ForceMoments<WorldSpace> const& ext_forces_1,
						  VelocityMoments<WorldSpace> const& vm_2, ForceMoments<WorldSpace> const& ext_forces_2,
						  si::Time dt) override;

	// Constraint API
	void
	calculated_constraint_forces (ConstraintForces const&) override;

	// Element API
	void
	flow_current (si::Time dt) override;

  private:
	std::optional<AngularSpringConstraint>					_spring_constraint;
	std::variant<std::monostate, double, TorqueEfficacy>	_efficiency_efficacy;
	si::Angle												_backlash;
	si::Angle												_setpoint			{ 0_deg };
	si::AngularVelocity										_angular_velocity;
	si::Torque												_torque;
	si::Power												_power_loss;
	SigmoidalTemperatureFailure								_failure_model;
};


/*
 * Global functions
 */


/**
 * Return an action torque function of angular error and servo operating voltage.
 *
 * \param	stall_torque_potential
 *			Servo stall torque per unit voltage.
 */
constexpr auto
servo_action_torque (TorquePotential stall_torque_potential)
{
	return [=] ([[maybe_unused]] si::Angle error, si::Voltage voltage) {
		return stall_torque_potential * voltage;
	};
}


/**
 * Return a drag torque function of angular velocity and servo operating voltage.
 *
 * \param	stall_torque_potential
 *			Servo stall torque per unit voltage.
 * \param	unloaded_angular_velocity_potential
 *			Unloaded servo arm velocity per unit voltage.
 */
constexpr auto
servo_drag_torque (TorquePotential stall_torque_potential, AngularVelocityPotential unloaded_angular_velocity_potential)
{
	return [=] (si::AngularVelocity arm_velocity, si::Voltage) {
		// Since unloaded angular velocity is maximum velocity, it's achieved when acting force and drag force are equally opposite.
		// So maximum drag == stall_torque_potential. Minimum drag is 0, when arm is not moving. Assume drag linear dependency on velocity
		// and compute current drag:
		return arm_velocity / unloaded_angular_velocity_potential * stall_torque_potential;
	};
}


/**
 * Return typical 9-gram servo constraint.
 *
 * \param	scale
 *			Scales up the torque, and scales down the speed.
 */
std::unique_ptr<AngularServoConstraint>
make_standard_9gram_servo_constraint (HingePrecalculation&, float scale = 1.0);

} // namespace xf::rigid_body

#endif

