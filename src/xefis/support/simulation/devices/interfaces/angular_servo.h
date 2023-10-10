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

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__INTERFACES__ANGULAR_SERVO_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__INTERFACES__ANGULAR_SERVO_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf::sim {

enum class ServoOrientation: int8_t
{
	Normal		= +1,
	Reversed	= -1,
};

}; // namespace xf::sim


namespace xf::sim::interfaces {

/**
 * Common interface for servomechanisms.
 */
class AngularServo
{
  public:
	// Dtor
	virtual
	~AngularServo() = default;

	/**
	 * Set servo setpoint in range [0, 1].
	 * Values outside allowed range should be clamped.
	 */
	virtual void
	set_setpoint (si::Angle) = 0;
};

} // namespace xf::sim::interfaces

#endif

