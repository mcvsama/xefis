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

// Standard:
#include <cstddef>
#include <memory>


namespace xf::sim {

class AngularServo:
	public interfaces::AngularServo,
	public rigid_body::Body
	// TODO and electrial device
{
  public:
	using Resolution = decltype (1 / 1_rad);

  public:
	/**
	 * Ctor
	 * \param	AngularVelocityPotential, TorquePotential
	 *			See AngularServoConstraint.
	 * \param	resolution
	 *			Number of recognized steps per radian of movement.
	 * \param	MassMoments
	 *			Servo mass moments.
	 */
	explicit
	AngularServo (rigid_body::AngularServoConstraint&, Resolution resolution_per_radian, MassMoments<rigid_body::BodySpace> const&);

	// Servo API
	void
	set_setpoint (si::Angle) override;

  private:
	rigid_body::AngularServoConstraint&	_constraint;
	Resolution							_resolution;
};


/*
 * Global functions
 */


std::unique_ptr<AngularServo>
make_standard_9gram_servo (rigid_body::AngularServoConstraint& constraint);

} // namespace xf::sim

#endif

