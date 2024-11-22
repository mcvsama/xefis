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
#include <xefis/support/control/pid_controller.h>
#include <xefis/support/simulation/components/resistor.h>
#include <xefis/support/simulation/constraints/angular_motor_constraint.h>
#include <xefis/support/simulation/constraints/hinge_precalculation.h>
#include <xefis/support/simulation/devices/interfaces/angular_servo.h>
#include <xefis/support/simulation/failure/sigmoidal_temperature_failure.h>
#include <xefis/support/simulation/rigid_body/body.h>

// Neutrino:
#include <neutrino/range.h>

// Standard:
#include <cstddef>
#include <memory>
#include <numeric>
#include <optional>
#include <utility>
#include <variant>


namespace xf::rigid_body {

// Typical parameters describing RC model servos:
using AngularVelocityPotential	= decltype (1_rad / 1_s / 1_V);
using TorquePotential			= decltype (1_Nm / 1_V);

// Typical microservo angular velocity potential:
static constexpr auto k9gramAngularVelocityPotential	= 60_deg / 0.15_s / 6_V;
static constexpr auto kStandardAngularVelocityPotential	= 60_deg / 0.20_s / 6_V;

// Typical microservo torque potential:
static constexpr auto k9gramTorquePotential				= 0.144_Nm / 6_V;
static constexpr auto kStandardTorquePotential			= 0.4_Nm / 6_V;


/**
 * Simplfied angular servomechanism constraint and electric device to use in
 * electrical and n-body simulators. Acts with a torque like a servo to make the arm
 * move to a specified set point angle.
 */
class AngularServoConstraint:
	public xf::sim::interfaces::AngularServo,
	public Constraint,
	public electrical::Resistor
{
  public:
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
	 *			If error (abs (actual - setpoint)) is greater or equal to backlash, normal torque will be used.
	 *			Otherwise, the servo will not act at all.
	 */
	explicit
	AngularServoConstraint (HingePrecalculation&, Range<si::Angle> angle_range, si::Angle backlash, AngularVelocityPotential, TorquePotential);

	[[nodiscard]]
	xf::sim::ServoOrientation
	orientation() const noexcept override
		{ return _orientation; }

	void
	set_orientation (xf::sim::ServoOrientation const orientation) override
		{ _orientation = orientation; }

	/**
	 * Return angle range.
	 */
	[[nodiscard]]
	Range<si::Angle>
	angle_range() const noexcept
		{ return _angle_range; }

	/**
	 * Return servo setpoint.
	 */
	[[nodiscard]]
	si::Angle
	setpoint() const noexcept override
		{ return _setpoint; }

	/**
	 * Set servo setpoint.
	 */
	void
	set_setpoint (si::Angle const setpoint) override
		{ _setpoint = std::to_underlying (_orientation) * std::clamp (setpoint, _angle_range.min(), _angle_range.max()); }

	/**
	 * Set electrical efficiency.
	 * Setting this invalidates value set by set_efficacy().
	 *
	 * \param	efficiency_factor
	 *			Must be in range 0…1.
	 */
	void
	set_efficiency (double efficiency_factor)
		{ _effic = efficiency_factor; }

	/**
	 * Set electrical efficacy.
	 * Setting this invalidates value set by set_efficiency().
	 *
	 * \param	efficacy
	 *			Torque per power (Nm per Watt).
	 */
	void
	set_efficacy (TorqueEfficacy efficacy)
		{ _effic = efficacy; }

	/**
	 * Return current relative arm velocity.
	 */
	[[nodiscard]]
	si::AngularVelocity
	arm_angular_velocity() const
		{ return _arm_angular_velocity; }

	/**
	 * Return current arm torque.
	 */
	[[nodiscard]]
	si::Torque
	arm_torque() const
		{ return _arm_torque; }

	// Constraint API
	void
	initialize_step (si::Time dt) override;

  protected:
	// Constraint API
	ConstraintForces
	do_constraint_forces (VelocityMoments<WorldSpace> const& vm_1, VelocityMoments<WorldSpace> const& vm_2, si::Time dt) override;

	// Constraint API
	void
	calculated_constraint_forces (ConstraintForces const&, si::Time dt) override;

	// Element API
	void
	flow_current (si::Time dt) override;

  private:
	void
	update_velocity_and_torque();

	void
	update_pid_controller (si::Time const dt);

  private:
	HingePrecalculation&									_hinge;
	xf::sim::ServoOrientation								_orientation			{ xf::sim::ServoOrientation::Normal };
	xf::PIDController<si::Angle, double>					_pid_controller;
	Range<si::Angle>										_angle_range;
	si::Angle												_backlash;
	si::Angle												_setpoint				{ _angle_range.midpoint() };
	std::variant<std::monostate, double, TorqueEfficacy>	_effic;
	si::Power												_power_loss				{ 0_W };
	SigmoidalTemperatureFailure								_failure_model;
	AngularVelocityPotential								_angular_velocity_potential;
	TorquePotential											_torque_potential;
	AngularMotorConstraint									_motor_constraint;
	si::AngularVelocity										_arm_angular_velocity	{ 0_radps };
	si::Torque												_arm_torque				{ 0_Nm };
};


/*
 * Global functions
 */


/**
 * Return standard servo constraint.
 *
 * \param	scale
 *			Scales up the torque, and scales down the speed.
 */
std::unique_ptr<AngularServoConstraint>
make_standard_servo_constraint (HingePrecalculation&, float scale = 1.0);

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

