/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>
#include <functional>

// System:
#include <linux/joystick.h>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "joystick.h"


JoystickInput::JoystickInput (QDomElement const& config)
{
	QString device_path;
	bool found_path = false;
	bool found_device = false;

	for (QDomElement& e: config)
	{
		if (e == "device")
		{
			if (found_device)
				throw Xefis::Exception ("only once <device> supported in configuration for the JoystickInput module");
			found_device = true;

			device_path = e.text();
		}
		else if (e == "path")
		{
			if (found_path)
				throw Xefis::Exception ("only once <path> supported in configuration for the JoystickInput module");
			found_path = true;

			_prop_path = e.text();
		}
		else
			throw Xefis::Exception (QString ("unsupported config element for JoystickInput module: <%1>").arg (e.tagName()).toStdString());
	}

	_device = new QFile (device_path, this);
	if (!_device->open (QFile::ReadOnly))
		throw Xefis::Exception (("could not open device file: " + device_path).toStdString());
	_notifier = new QSocketNotifier (_device->handle(), QSocketNotifier::Read, this);
	_notifier->setEnabled (true);
	QObject::connect (_notifier, SIGNAL (activated (int)), this, SLOT (read()));

	// Support max 256 axes/buttons:
	_buttons.resize (256, nullptr);
	_axes.resize (256, nullptr);
}


JoystickInput::~JoystickInput()
{
	for (auto& prop: _buttons)
		delete prop;
	for (auto& prop: _axes)
		delete prop;
}


void
JoystickInput::read()
{
	::js_event ev;
	ssize_t n = ::read (_device->handle(), &ev, sizeof (ev));
	if (n == sizeof (ev))
	{
		switch (ev.type)
		{
			case JS_EVENT_BUTTON:
			case JS_EVENT_BUTTON + JS_EVENT_INIT:
			{
				if (ev.number >= _buttons.size())
					break;

				Buttons::value_type prop;
				if (ev.type == JS_EVENT_BUTTON + JS_EVENT_INIT)
				{
					QString path = QString ("%1/button/%2").arg (_prop_path).arg (ev.number);
					prop = _buttons[ev.number] = new Xefis::Property<bool> (path.toStdString());
				}
				else
				{
					prop = _buttons[ev.number];
					if (!prop)
						break;
				}
				prop->write (ev.value);
				break;
			}

			case JS_EVENT_AXIS:
			case JS_EVENT_AXIS + JS_EVENT_INIT:
			{
				if (ev.number >= _axes.size())
					break;
				Axes::value_type prop;
				if (ev.type == JS_EVENT_AXIS + JS_EVENT_INIT)
				{
					QString path = QString ("%1/axis/%2").arg (_prop_path).arg (ev.number);
					prop = _axes[ev.number] = new Xefis::Property<float> (path.toStdString());
				}
				else
				{
					prop = _axes[ev.number];
					if (!prop)
						break;
				}
				prop->write (ev.value / 32767.0);
				break;
			}
		}
	}
}

