/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__INPUT__JOYSTICK_H__INCLUDED
#define XEFIS__INPUT__JOYSTICK_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/input.h>
#include <xefis/utility/qdom.h>


class JoystickInput:
	public QObject,
	public Xefis::Input
{
	Q_OBJECT

	typedef std::vector<Xefis::PropertyBoolean*>	Buttons;
	typedef std::vector<Xefis::PropertyFloat*>		Axes;

  public:
	// Ctor
	JoystickInput (Xefis::ModuleManager*, QDomElement const& config);

	// Dtor
	~JoystickInput();

  private slots:
	/**
	 * Try to open input device.
	 */
	void
	open_device();

	/**
	 * Read event from the device.
	 */
	void
	read();

  private:
	QString				_prop_path		= "/joystick";
	QString				_device_path;
	QFile*				_device			= nullptr;
	QSocketNotifier*	_notifier		= nullptr;
	QTimer*				_reopen_timer	= nullptr;
	Buttons				_buttons;
	Axes				_axes;
};

#endif
