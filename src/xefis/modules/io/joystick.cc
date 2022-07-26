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

// Local:
#include "joystick.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>
#include <neutrino/qt/qdom.h>
#include <neutrino/qt/qdom_iterator.h>
#include <neutrino/stdexcept.h>

// System:
#include <linux/joystick.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Standard:
#include <cstddef>
#include <functional>


JoystickInput::Button::Button (QDomElement const&, xf::ModuleOut<bool>& socket):
	_socket (socket)
{ }


xf::ModuleOut<bool>&
JoystickInput::Button::socket()
{
	return _socket;
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
	_socket = xf::nil;
}


inline void
JoystickInput::Button::set_value (float value)
{
	_socket = value;
}


JoystickInput::Axis::Axis (QDomElement const& axis_element, xf::ModuleOut<double>& socket, xf::ModuleOut<si::Angle>& angle_socket, xf::Range<si::Angle>& angle_range):
	Axis (axis_element, socket, angle_socket, angle_range, { }, { })
{ }


JoystickInput::Axis::Axis (QDomElement const& axis_element, xf::ModuleOut<double>& socket, xf::ModuleOut<si::Angle>& angle_socket, xf::Range<si::Angle>& angle_range,
						   std::optional<HandlerID> up_button_id, std::optional<HandlerID> down_button_id):
	_socket (socket),
	_angle_socket (angle_socket),
	_angle_range (angle_range),
	_up_button_id (up_button_id),
	_down_button_id (down_button_id)
{
	for (QDomElement const& v: xf::iterate_sub_elements (axis_element))
	{
		if (v == "center")
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
			for (QDomElement const& w: xf::iterate_sub_elements (v))
			{
				if (w == "minimum")
					_output_minimum = w.text().toFloat();
				else if (w == "maximum")
					_output_maximum = w.text().toFloat();
			}
		}
		else
			throw xf::UnexpectedDomElement (v);
	}
}


xf::ModuleOut<double>&
JoystickInput::Axis::socket()
{
	return _socket;
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
					set_value ((value > 0) ? +1.0 : 0.0);
				else if (handler_id == *_down_button_id)
					set_value ((value > 0) ? -1.0 : 0.0);
			}
			break;

		default:
			break;
	}
}


void
JoystickInput::Axis::reset()
{
	_socket = xf::nil;
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
	value = xf::renormalize (value, xf::Range (-1.f, 1.f), xf::Range (_output_minimum, _output_maximum));

	_socket = value;
	_angle_socket = xf::renormalize (value, { -1.0f, +1.0f }, _angle_range);
}


JoystickInput::JoystickInput (QDomElement const& config, xf::Logger const& logger, std::string_view const& instance):
	JoystickInputIO (instance),
	_logger (logger.with_scope (std::string (kLoggerScope) + "#" + instance))
{
	for (std::size_t handler_id = 0; handler_id < kMaxEventID; ++handler_id)
		_button_sockets[handler_id] = std::make_unique<xf::ModuleOut<bool>> (&_io, "buttons/" + std::to_string (handler_id));

	for (std::size_t handler_id = 0; handler_id < kMaxEventID; ++handler_id)
		_axis_sockets[handler_id] = std::make_unique<xf::ModuleOut<double>> (&_io, "axes/" + std::to_string (handler_id));

	for (std::size_t handler_id = 0; handler_id < kMaxEventID; ++handler_id)
	{
		_angle_axis_sockets[handler_id] = std::make_unique<xf::ModuleOut<si::Angle>> (&_io, "axes(angle)/" + std::to_string (handler_id));
		_angle_axis_ranges[handler_id] = { -45_deg, +45_deg };
	}

	for (QDomElement const& e: xf::iterate_sub_elements (config))
	{
		if (e == "axis")
		{
			if (e.hasAttribute ("id"))
			{
				auto id = e.attribute ("id").toUInt();

				if (id < _handlers.size())
				{
					auto& socket = *_axis_sockets[id];
					auto& angle_socket = *_angle_axis_sockets[id];
					auto& angle_range =_angle_axis_ranges[id];
					_handlers[id].push_back (std::make_shared<Axis> (e, socket, angle_socket, angle_range));

					if (e.hasAttribute ("up-button-id") && e.hasAttribute ("down-button-id"))
					{
						unsigned int u_id = e.attribute ("up-button-id").toUInt();
						unsigned int d_id = e.attribute ("down-button-id").toUInt();

						if (u_id < _handlers.size() && d_id < _handlers.size())
						{
							auto axis = std::make_shared<Axis> (e, socket, angle_socket, angle_range, u_id, d_id);

							_handlers[u_id].push_back (axis);
							_handlers[d_id].push_back (axis);
						}
					}
				}
				else
					throw xf::BadDomAttribute (e, "id");
			}
			else
				throw xf::MissingDomAttribute (e, "id");
		}
		else if (e == "button")
		{
			if (e.hasAttribute ("id"))
			{
				auto id = e.attribute ("id").toUInt();

				if (id < _handlers.size())
					_handlers[id].push_back (std::make_shared<Button> (e, *_button_sockets[id]));
				else
					throw xf::BadDomAttribute (e, "id");
			}
			else
				throw xf::MissingDomAttribute (e, "id");
		}
		else if (e == "device")
			_device_path = e.text().toStdString();
		else
			throw xf::UnexpectedDomElement (e);
	}

	if (!_device_path)
		throw xf::MissingDomElement (config, "device");

	_reopen_timer = std::make_unique<QTimer> (this);
	_reopen_timer->setInterval (500);
	_reopen_timer->setSingleShot (true);
	QObject::connect (_reopen_timer.get(), SIGNAL (timeout()), this, SLOT (open_device()));
}


void
JoystickInput::initialize()
{
	open_device();
}


void
JoystickInput::open_device()
{
	try {
		_logger << "Opening device " << *_device_path << std::endl;

		_device = ::open (_device_path->c_str(), O_RDONLY | O_NDELAY);

		if (_device < 0)
		{
			_logger << "Could not open device file " << *_device_path << ": " << strerror (errno) << std::endl;
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
		_logger << "Failure detected, closing device " << *_device_path << std::endl;

	_failure_count += 1;
	_notifier.reset();
	::close (_device);

	restart();
}


void
JoystickInput::restart()
{
	if (_io.restart_on_failure)
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
						_available_buttons.insert (handler_id);
				// fallthrough
				case JS_EVENT_BUTTON:
					event_type = EventType::ButtonEvent;
					*_button_sockets[handler_id] = ev.value;
					break;

				case JS_EVENT_AXIS + JS_EVENT_INIT:
					if (ev.type == JS_EVENT_AXIS + JS_EVENT_INIT)
						_available_axes.insert (handler_id);
				// fallthrough
				case JS_EVENT_AXIS:
					event_type = EventType::AxisEvent;
					break;
			}

			for (auto& handler: _handlers[handler_id])
				handler->handle (event_type, handler_id, ev.value);
		}
		else
			_logger << "Joystick event with ID " << handler_id << " greater than max supported " << kMaxEventID << std::endl;
	}
}


void
JoystickInput::reset_sockets()
{
	for (auto& handlers: _handlers)
		for (auto& handler: handlers)
			handler->reset();

	for (auto& button: _button_sockets)
		*button = xf::nil;

	for (auto& axis: _axis_sockets)
		*axis = xf::nil;
}

