/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__IO__JOYSTICK_H__INCLUDED
#define XEFIS__MODULES__IO__JOYSTICK_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtCore/QObject>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/module.h>
#include <xefis/utility/qdom.h>


class JoystickInput:
	public QObject,
	public xf::Module
{
	Q_OBJECT

	static constexpr size_t kMaxID = 256;

	struct Button
	{
		void
		set_value (float value);

		xf::PropertyBoolean	user_defined_property;
	};

	struct Axis
	{
		void
		set_value (float value);

		xf::PropertyFloat	user_defined_property;
		float				center			= 0.f;
		float				dead_zone		= 0.f;
		float				reverse			= 1.f;
		float				scale			= 1.f;
		float				power			= 1.f;
		float				output_minimum	= -1.f;
		float				output_maximum	= +1.f;
	};

	typedef std::vector<std::vector<Button>>	Buttons;
	typedef std::vector<std::vector<Axis>>		Axes;

	typedef std::vector<xf::PropertyBoolean>	ButtonProperties;
	typedef std::vector<xf::PropertyFloat>		AxisProperties;

  public:
	// Ctor
	JoystickInput (xf::ModuleManager*, QDomElement const& config);

  private slots:
	/**
	 * Try to open input device.
	 */
	void
	open_device();

	/**
	 * Close device after failure is detected.
	 */
	void
	failure();

	/**
	 * Start reopen timer.
	 */
	void
	restart();

	/**
	 * Read event from the device.
	 */
	void
	read();

  private:
	/**
	 * Set all properties to nil.
	 */
	void
	reset_properties();

  private:
	QString					_prop_path			= "/joystick";
	QString					_device_path;
	int						_device				= 0;
	Unique<QSocketNotifier>	_notifier;
	Unique<QTimer>			_reopen_timer;
	Buttons					_buttons;
	Axes					_axes;
	// TODO resize
	ButtonProperties		_button_properties;
	AxisProperties			_axis_properties;
	unsigned int			_failure_count		= 0;
	bool					_restart_on_failure	= true;
};

#endif
