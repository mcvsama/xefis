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


XEFIS_REGISTER_MODULE_CLASS ("io/joystick", JoystickInput)


constexpr size_t JoystickInput::kMaxID;


JoystickInput::Button::Button (QDomElement const& button_element)
{
	for (QDomElement& v: button_element)
	{
		if (v == "path")
			_user_defined_property.set_path (xf::PropertyPath (v.text()));
	}
}


void
JoystickInput::Button::handle (EventType event_type, HandlerID, int32_t value)
{
	if (event_type == EventType::ButtonEvent)
		set_value (value);
}


void
JoystickInput::Button::reset()
{
	_user_defined_property.set_nil();
}


inline void
JoystickInput::Button::set_value (float value)
{
	if (_user_defined_property.configured())
		_user_defined_property = value;
}


JoystickInput::Axis::Axis (QDomElement const& axis_element):
	Axis (axis_element, { }, { })
{ }


JoystickInput::Axis::Axis (QDomElement const& axis_element, Optional<HandlerID> up_button_id, Optional<HandlerID> down_button_id):
	_up_button_id (up_button_id),
	_down_button_id (down_button_id)
{
	for (QDomElement& v: axis_element)
	{
		if (v == "path")
			_user_defined_property.set_path (xf::PropertyPath (v.text()));
		else if (v == "center")
			_center = v.text().toFloat();
		else if (v == "dead-zone")
			_dead_zone = v.text().toFloat();
		else if (v == "reverse")
			_reverse = -1.f;
		else if (v == "scale")
			_scale = v.text().toFloat();
		else if (v == "power")
			_power = v.text().toFloat();
		else if (v == "output")
		{
			for (QDomElement& w: v)
			{
				if (w == "minimum")
					_output_minimum = w.text().toFloat();
				else if (w == "maximum")
					_output_maximum = w.text().toFloat();
			}
		}
		else
			throw xf::BadDomElement (v);
	}
}


void
JoystickInput::Axis::handle (EventType event_type, HandlerID handler_id, int32_t value)
{
	switch (event_type)
	{
		case EventType::AxisEvent:
			if (!_up_button_id && !_down_button_id)
				set_value (value / 32767.0);
			break;

		case EventType::ButtonEvent:
			if (_up_button_id && _down_button_id)
			{
				if (handler_id == *_up_button_id)
					set_value (value > 0);
				else if (handler_id == *_down_button_id)
					set_value (-(value > 0));
			}
			break;

		default:
			break;
	}
}


void
JoystickInput::Axis::reset()
{
	_user_defined_property.set_nil();
}


inline void
JoystickInput::Axis::set_value (float value)
{
	// Center:
	value -= _center;
	// Remove dead zone:
	if (std::abs (value) < _dead_zone)
		value = 0.f;
	else
		value = value - xf::sgn (value) * _dead_zone;
	// Reverse:
	value *= _reverse;
	// Scale:
	value *= _scale;
	// Power:
	value = xf::sgn (value) * std::pow (std::abs (value), _power);
	// Renormalize from standard [-1.0, 1.0]:
	value = xf::renormalize (value, xf::Range<float> (-1.f, 1.f), xf::Range<float> (_output_minimum, _output_maximum));

	if (_user_defined_property.configured())
		_user_defined_property = value;
}


JoystickInput::JoystickInput (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	// Support max kMaxID axes/buttons:
	_handlers.resize (kMaxID);
	_button_properties.resize (kMaxID);
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
			if (e.hasAttribute ("id"))
			{
				unsigned int id = e.attribute ("id").toUInt();

				if (id < _handlers.size())
					_handlers[id].push_back (std::make_shared<Axis> (e));
			}
			else if (e.hasAttribute ("up-button-id") && e.hasAttribute ("down-button-id"))
			{
				unsigned int u_id = e.attribute ("up-button-id").toUInt();
				unsigned int d_id = e.attribute ("down-button-id").toUInt();

				if (u_id < _handlers.size() && d_id < _handlers.size())
				{
					auto axis = std::make_shared<Axis> (e, u_id, d_id);

					_handlers[u_id].push_back (axis);
					_handlers[d_id].push_back (axis);
				}
			}
			else
				throw xf::BadDomElement (e);
		}
		else if (e == "button")
		{
			if (e.hasAttribute ("id"))
			{
				unsigned int id = e.attribute ("id").toUInt();

				if (id < _handlers.size())
					_handlers[id].push_back (std::make_shared<Button> (e));
			}
			else
				throw xf::MissingDomAttribute (e, "id");
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
		HandlerID handler_id = ev.number;

		if (handler_id < _handlers.size())
		{
			EventType event_type = EventType::Unknown;

			switch (ev.type)
			{
				case JS_EVENT_BUTTON + JS_EVENT_INIT:
					if (ev.type == JS_EVENT_BUTTON + JS_EVENT_INIT)
						_button_properties[handler_id].set_path (xf::PropertyPath (QString ("%1/button/%2").arg (_prop_path).arg (handler_id)));
				// fallthrough
				case JS_EVENT_BUTTON:
					event_type = EventType::ButtonEvent;
					if (_button_properties[handler_id].configured())
						_button_properties[handler_id] = ev.value;
					break;

				case JS_EVENT_AXIS + JS_EVENT_INIT:
					if (ev.type == JS_EVENT_AXIS + JS_EVENT_INIT)
						_axis_properties[handler_id].set_path (xf::PropertyPath (QString ("%1/axis/%2").arg (_prop_path).arg (handler_id)));
				// fallthrough
				case JS_EVENT_AXIS:
					event_type = EventType::AxisEvent;
					if (_axis_properties[handler_id].configured())
						_axis_properties[handler_id] = ev.value / 32767.0;
					break;
			}

			for (auto& handler: _handlers[handler_id])
				handler->handle (event_type, handler_id, ev.value);
		}
		else
			log() << "Joystick event with ID " << handler_id << " greater than max supported " << kMaxID << std::endl;
	}
}


void
JoystickInput::reset_properties()
{
	for (auto& handlers: _handlers)
		for (auto& handler: handlers)
			handler->reset();

	for (auto& button: _button_properties)
		button.set_nil();

	for (auto& axis: _axis_properties)
		axis.set_nil();
}

