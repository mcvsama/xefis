/* vim:ts=4
 *
 * Copyleft 2008…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CONFIGS__CTHULHU_GCS__WARTHOG_STICK_H__INCLUDED
#define XEFIS__CONFIGS__CTHULHU_GCS__WARTHOG_STICK_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/modules/io/joystick.h>


class WarthogStick: public JoystickInput
{
  public:
	/*
	 * Output
	 */

	v2::PropertyOut<double>&	pitch_axis	= axis (3);
	v2::PropertyOut<double>&	roll_axis	= axis (4);
	v2::PropertyOut<bool>&		fire_button	= button (5);

  public:
	using JoystickInput::JoystickInput;
};

#endif

