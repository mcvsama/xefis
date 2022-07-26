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

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__SERVO_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__SERVO_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf::sim {

/**
 * Common interface for servomechanisms.
 */
class Servo
{
  public:
	// Dtor
	virtual
	~Servo() = default;

	/**
	 * Set servo setpoint.
	 * Valid values are [0…1].
	 */
	virtual void
	set_setpoint (double value) = 0;
};

} // namespace xf::sim

#endif

