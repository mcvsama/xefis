/* vim:ts=4
 *
 * Copyleft 2022  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__SIMULATION__VIRTUAL_JOYSTICK_H__INCLUDED
#define XEFIS__MODULES__SIMULATION__VIRTUAL_JOYSTICK_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/ui/widget.h>


class VirtualJoystickWidget;
class VirtualLinearWidget;


class VirtualJoystickIO: public xf::Module
{
  public:
	/*
	 * Output
	 */

	xf::ModuleOut<double>	x_axis		{ this, "axis/x" };
	xf::ModuleOut<double>	y_axis		{ this, "axis/y" };
	xf::ModuleOut<double>	rudder		{ this, "rudder" };
	xf::ModuleOut<double>	throttle	{ this, "throttle" };

  public:
	using xf::Module::Module;
};


class VirtualJoystick: public VirtualJoystickIO
{
  public:
	// Ctor
	explicit
	VirtualJoystick (std::string_view const& instance = {});

	QWidget*
	widget() const noexcept;

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	VirtualJoystickIO&		_io { *this };
	xf::Widget*				_widget;
	VirtualJoystickWidget*	_joystick_widget;
	VirtualLinearWidget*	_throttle_widget;
	VirtualLinearWidget*	_rudder_widget;
};


inline QWidget*
VirtualJoystick::widget() const noexcept
{
	return _widget;
}

#endif
