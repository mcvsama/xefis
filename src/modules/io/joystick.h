/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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
	public Xefis::Module
{
	Q_OBJECT

	struct Button
	{
		void
		set_value (float value);

		Xefis::PropertyBoolean	prop;
		Xefis::PropertyBoolean	alt_prop;
	};

	struct Axis
	{
		void
		set_value (float value);

		Xefis::PropertyFloat	prop;
		Xefis::PropertyFloat	alt_prop;
		float					center			= 0.f;
		float					dead_zone		= 0.f;
		float					reverse			= 1.f;
		float					scale			= 1.f;
		float					power			= 1.f;
		float					output_minimum	= -1.f;
		float					output_maximum	= +1.f;
	};

	typedef std::vector<Shared<Button>>	Buttons;
	typedef std::vector<Shared<Axis>>	Axes;

  public:
	// Ctor
	JoystickInput (Xefis::ModuleManager*, QDomElement const& config);

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
	QString					_prop_path		= "/joystick";
	QString					_device_path;
	int						_device			= 0;
	Unique<QSocketNotifier>	_notifier;
	Unique<QTimer>			_reopen_timer;
	Buttons					_buttons;
	Axes					_axes;
	unsigned int			_failure_count	= 0;
};

#endif
