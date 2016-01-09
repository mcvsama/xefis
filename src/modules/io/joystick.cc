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
#include <xefis/core/stdexcept.h>
#include <xefis/utility/numeric.h>

// Local:
#include "joystick.h"


XEFIS_REGISTER_MODULE_CLASS ("io/joystick", JoystickInput);


constexpr size_t JoystickInput::kMaxID;


inline void
JoystickInput::Button::set_value (float value)
{
	if (user_defined_property.configured())
		user_defined_property = value;
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
		value = value - xf::sgn (value) * dead_zone;
	// Reverse:
	value *= reverse;
	// Scale:
	value *= scale;
	// Power:
	value = xf::sgn (value) * std::pow (std::abs (value), power);
	// Renormalize from standard [-1.0, 1.0]:
	value = xf::renormalize (value, xf::Range<float> (-1.f, 1.f), xf::Range<float> (output_minimum, output_maximum));

	if (user_defined_property.configured())
		user_defined_property = value;
}


JoystickInput::JoystickInput (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	// Support max kMaxID axes/buttons:
	_buttons.resize (kMaxID);
	_button_properties.resize (kMaxID);
	_axes.resize (kMaxID);
	_axis_properties.resize (kMaxID);

	parse_settings (config, {
		{ "device", _device_path, true },
		{ "path", _prop_path, true },
		{ "restart-on-failure", _restart_on_failure, false },
	});

	for (QDomElement& e: config)
	{
		if (e == "axis")
		{
			unsigned int id = e.attribute ("id").toUInt();
			if (id < _axes.size())
			{
				Axis axis;

				for (QDomElement& v: e)
				{
					if (v == "path")
						axis.user_defined_property.set_path (xf::PropertyPath (v.text()));
					else if (v == "center")
						axis.center = v.text().toFloat();
					else if (v == "dead-zone")
						axis.dead_zone = v.text().toFloat();
					else if (v == "reverse")
						axis.reverse = -1.f;
					else if (v == "scale")
						axis.scale = v.text().toFloat();
					else if (v == "power")
						axis.power = v.text().toFloat();
					else if (v == "output")
					{
						for (QDomElement& w: v)
						{
							if (w == "minimum")
								axis.output_minimum = w.text().toFloat();
							else if (w == "maximum")
								axis.output_maximum = w.text().toFloat();
						}
					}
				}

				_axes[id].push_back (std::move (axis));
			}
		}
		else if (e == "button")
		{
			unsigned int id = e.attribute ("id").toUInt();
			if (id < _buttons.size())
			{
				Button button;

				for (QDomElement& v: e)
				{
					if (v == "path")
						button.user_defined_property.set_path (xf::PropertyPath (v.text()));
				}

				_buttons[id].push_back (std::move (button));
			}
		}
		else
		{
			if (e != "settings")
				throw xf::BadDomElement (e);
		}
	}

	_reopen_timer = std::make_unique<QTimer> (this);
	_reopen_timer->setInterval (500);
	_reopen_timer->setSingleShot (true);
	QObject::connect (_reopen_timer.get(), SIGNAL (timeout()), this, SLOT (open_device()));

	open_device();
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
			_notifier = std::make_unique<QSocketNotifier> (_device, QSocketNotifier::Read, this);
			_notifier->setEnabled (true);
			QObject::connect (_notifier.get(), SIGNAL (activated (int)), this, SLOT (read()));
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
	_notifier.reset();
	::close (_device);

	restart();
}


void
JoystickInput::restart()
{
	if (_restart_on_failure)
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
				auto const button_id = ev.number;
				bool const value = ev.value;

				if (button_id >= _buttons.size())
					break;

				if (ev.type == JS_EVENT_BUTTON + JS_EVENT_INIT)
				{
					_button_properties[button_id].set_path (xf::PropertyPath (QString ("%1/button/%2").arg (_prop_path).arg (button_id)));
					_button_properties[button_id] = value;
				}

				for (auto& button: _buttons[button_id])
					button.set_value (value);
				break;
			}

			case JS_EVENT_AXIS:
			case JS_EVENT_AXIS + JS_EVENT_INIT:
			{
				auto const axis_id = ev.number;
				float const value = ev.value / 32767.0;

				if (axis_id >= _axes.size())
					break;

				if (ev.type == JS_EVENT_AXIS + JS_EVENT_INIT)
				{
					_axis_properties[axis_id].set_path (xf::PropertyPath (QString ("%1/axis/%2").arg (_prop_path).arg (axis_id)));
					_axis_properties[axis_id] = value;
				}

				for (auto& axis: _axes[axis_id])
					axis.set_value (value);
				break;
			}
		}
	}
}


void
JoystickInput::reset_properties()
{
	for (auto& buttons: _buttons)
		for (auto& button: buttons)
			button.user_defined_property.set_nil();

	for (auto& button: _button_properties)
		button.set_nil();

	for (auto& axes: _axes)
		for (auto& axis: axes)
			axis.user_defined_property.set_nil();

	for (auto& axis: _axis_properties)
		axis.set_nil();
}

