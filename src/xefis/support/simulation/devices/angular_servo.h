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

// Standard:
#include <cstddef>
#include <memory>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/constraints/hinge_precalculation.h>
#include <xefis/support/simulation/devices/interfaces/servo.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/simulation/rigid_body/constraint.h>


namespace xf::sim {

class AngularServo: public rigid_body::Body
					// TODO and electrial device
{
  public:
	/**
	 * Ctor
	 * \param	AngularVelocityPotential, TorquePotential
	 *			See AngularServoConstraint.
	 * \param	MassMoments
	 *			Servo mass moments.
	 */
	explicit
	AngularServo (MassMoments<rigid_body::BodySpace> const&);
};


/*
 * Global functions
 */


std::unique_ptr<AngularServo>
make_standard_9gram_servo (Placement<rigid_body::WorldSpace, rigid_body::BodySpace> const& location);

} // namespace xf::sim

#endif

