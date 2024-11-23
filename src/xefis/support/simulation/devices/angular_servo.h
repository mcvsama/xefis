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

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__ANGULAR_SERVO_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__ANGULAR_SERVO_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/constraints/hinge_precalculation.h>
#include <xefis/support/simulation/constraints/angular_servo_constraint.h>
#include <xefis/support/simulation/devices/interfaces/angular_servo.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/simulation/rigid_body/constraint.h>
#include <xefis/support/ui/observation_widget.h>

// Standard:
#include <cstddef>
#include <memory>


namespace xf::sim {

class AngularServo:
	public interfaces::AngularServo,
	public rigid_body::Body,
	public HasObservationWidget
	// TODO and electrial device
{
  public:
	/**
	 * Ctor
	 * \param	AngularVelocityPotential, TorquePotential
	 *			See AngularServoConstraint.
	 * \param	resolution
	 *			Angle per step. Usually between 0.5° (digital) to 2° (analog).
	 * \param	MassMoments
	 *			Servo mass moments.
	 */
	explicit
	AngularServo (rigid_body::AngularServoConstraint&, si::Angle resolution, MassMoments<BodyCOM> const&);

	[[nodiscard]]
	si::Angle
	resolution() const noexcept
		{ return _resolution; }

	// Servo API
	[[nodiscard]]
	xf::sim::ServoOrientation
	orientation() const noexcept override
		{ return _constraint.orientation(); }

	// Servo API
	void
	set_orientation (xf::sim::ServoOrientation const orientation) override
		{ _constraint.set_orientation (orientation); }

	// Servo API
	[[nodiscard]]
	virtual si::Angle
	setpoint() const noexcept override
		{ return _constraint.setpoint(); }

	// Servo API
	void
	set_setpoint (si::Angle) override;

	[[nodiscard]]
	rigid_body::AngularServoConstraint const&
	constraint() const noexcept
		{ return _constraint; }

	[[nodiscard]]
	rigid_body::AngularServoConstraint&
	constraint() noexcept
		{ return _constraint; }

  private:
	rigid_body::AngularServoConstraint&	_constraint;
	si::Angle							_resolution;
};


/*
 * Global functions
 */


std::unique_ptr<AngularServo>
make_standard_servo (rigid_body::AngularServoConstraint& constraint, float scale = 1.0);

std::unique_ptr<AngularServo>
make_standard_9gram_servo (rigid_body::AngularServoConstraint& constraint);

} // namespace xf::sim

#endif

