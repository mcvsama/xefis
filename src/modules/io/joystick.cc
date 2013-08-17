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

// Standard:
#include <cstddef>
#include <functional>

// System:
#include <linux/joystick.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>

// Local:
#include "joystick.h"


XEFIS_REGISTER_MODULE_CLASS ("io/joystick", JoystickInput);


inline void
JoystickInput::Button::set_path (std::string const& path)
{
	prop = Xefis::PropertyBoolean (path);
}


inline void
JoystickInput::Button::set_value (float value)
{
	prop.write (value);
}


inline void
JoystickInput::Axis::set_path (std::string const& path)
{
	prop = Xefis::PropertyFloat (path);
}


inline void
JoystickInput::Axis::set_value (float value)
{
	// Center:
	value -= center;
	// Remove dead zone:
	if (std::abs (value) < dead_zone)
		value = 0.f;
	else
		value = value - Xefis::sgn (value) * dead_zone;
	// Reverse:
	value *= reverse;
	// Scale:
	value *= scale;
	// Power:
	value = Xefis::sgn (value) * std::pow (std::abs (value), power);

	prop.write (value);
}


JoystickInput::JoystickInput (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	// Support max 256 axes/buttons:
	_buttons.resize (256, nullptr);
	_axes.resize (256, nullptr);

	bool found_path = false;
	bool found_device = false;

	for (QDomElement& e: config)
	{
		if (e == "device")
		{
			if (found_device)
				throw Xefis::Exception ("only one <device> supported in configuration for the io/joystick module");
			found_device = true;

			_device_path = e.text();
		}
		else if (e == "path")
		{
			if (found_path)
				throw Xefis::Exception ("only one <path> supported in configuration for the io/joystick module");
			found_path = true;

			_prop_path = e.text();
		}
		else if (e == "axis")
		{
			unsigned int id = e.attribute ("id").toUInt();
			if (id < _axes.size())
			{
				float center = 0.f;
				float dead_zone = 0.f;
				float reverse = 1.f;
				float scale = 1.f;
				float power = 1.f;

				for (QDomElement& v: e)
				{
					if (v == "center")
						center = v.text().toFloat();
					else if (v == "dead-zone")
						dead_zone = v.text().toFloat();
					else if (v == "reverse")
						reverse = -1.f;
					else if (v == "scale")
						scale = v.text().toFloat();
					else if (v == "power")
						power = v.text().toFloat();
				}

				Axis* axis = new Axis();
				axis->center = center;
				axis->dead_zone = dead_zone;
				axis->reverse = reverse;
				axis->scale = scale;
				axis->power = power;
				_axes[id] = axis;
			}
		}
		else
			throw Xefis::Exception (QString ("unsupported config element for io/joystick module: <%1>").arg (e.tagName()).toStdString());
	}

	if (!found_path)
		throw Xefis::Exception ("config for the io/joystick module needs <path> element");
	if (!found_device)
		throw Xefis::Exception ("config for the io/joystick module needs <device> element");

	_reopen_timer = new QTimer (this);
	_reopen_timer->setInterval (500);
	_reopen_timer->setSingleShot (true);
	QObject::connect (_reopen_timer, SIGNAL (timeout()), this, SLOT (open_device()));

	open_device();
}


JoystickInput::~JoystickInput()
{
	for (auto& x: _buttons)
		delete x;
	for (auto& x: _axes)
		delete x;
}


void
JoystickInput::open_device()
{
	try {
		log() << "Opening device " << _device_path.toStdString() << std::endl;

		_device = ::open (_device_path.toStdString().c_str(), O_RDONLY | O_NDELAY);

		if (_device < 0)
		{
			log() << "Could not open device file " << _device_path.toStdString() << ": " << strerror (errno) << std::endl;
			restart();
		}
		else
		{
			_failure_count = 0;
			delete _notifier;
			_notifier = new QSocketNotifier (_device, QSocketNotifier::Read, this);
			_notifier->setEnabled (true);
			QObject::connect (_notifier, SIGNAL (activated (int)), this, SLOT (read()));
		}
	}
	catch (...)
	{
		failure();
	}
}


void
JoystickInput::failure()
{
	if (_failure_count <= 1)
		log() << "Failure detected, closing device " << _device_path.toStdString() << std::endl;

	_failure_count += 1;
	delete _notifier;
	_notifier = nullptr;
	::close (_device);

	restart();
}


void
JoystickInput::restart()
{
	_reopen_timer->start();
}


void
JoystickInput::read()
{
	::js_event ev;
	ssize_t n = ::read (_device, &ev, sizeof (ev));
	if (n < 0)
		failure();
	else if (n == sizeof (ev))
	{
		switch (ev.type)
		{
			case JS_EVENT_BUTTON:
			case JS_EVENT_BUTTON + JS_EVENT_INIT:
			{
				if (ev.number >= _buttons.size())
					break;

				Buttons::value_type button;
				if (ev.type == JS_EVENT_BUTTON + JS_EVENT_INIT)
				{
					button = _buttons[ev.number];
					if (!button)
						button = _buttons[ev.number] = new Button();
					QString path = QString ("%1/button/%2").arg (_prop_path).arg (ev.number);
					_buttons[ev.number]->set_path (path.toStdString());
				}
				else
				{
					button = _buttons[ev.number];
					if (!button)
						break;
				}
				button->set_value (ev.value);
				break;
			}

			case JS_EVENT_AXIS:
			case JS_EVENT_AXIS + JS_EVENT_INIT:
			{
				if (ev.number >= _axes.size())
					break;
				Axes::value_type axis;
				if (ev.type == JS_EVENT_AXIS + JS_EVENT_INIT)
				{
					axis = _axes[ev.number];
					if (!axis)
						axis = _axes[ev.number] = new Axis();
					QString path = QString ("%1/axis/%2").arg (_prop_path).arg (ev.number);
					_axes[ev.number]->set_path (path.toStdString());
				}
				else
				{
					axis = _axes[ev.number];
					if (!axis)
						break;
				}
				axis->set_value (ev.value / 32767.0);
				break;
			}
		}
	}

	signal_data_updated();
}


void
JoystickInput::reset_properties()
{
	for (Button* b: _buttons)
		if (b)
			b->prop.set_nil();
	for (Axis* a: _axes)
		if (a)
			a->prop.set_nil();
}

