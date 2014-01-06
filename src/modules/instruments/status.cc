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

// Qt:
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "status.h"
#include "status_widget.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/status", Status);


Status::MessageDefinition::Observation::Observation (QDomElement const& observe_element)
{
	QString fail_on_nil = observe_element.attribute ("fail-on-nil");
	if (observe_element.hasAttribute ("fail-on-nil") && fail_on_nil != "true" && fail_on_nil != "false")
		throw Xefis::Exception ("invalid value for attribute @fail-on-nil on <message> element - must be 'true' or 'false'");

	if (!observe_element.hasAttribute ("path"))
		throw Xefis::Exception ("missing @path property on <message> element");

	QString fail_on = observe_element.attribute ("fail-on");
	if (!observe_element.hasAttribute ("fail-on"))
		throw Xefis::Exception ("missing @fail-on property on <message> element");
	if (fail_on != "true" && fail_on != "false")
		throw Xefis::Exception ("invalid value for attribute @fail-on on <message> element - must be 'true' or 'false'");

	_observed_property.set_path (observe_element.attribute ("path").toStdString());
	_valid_state = fail_on != "true";
	_fail_on_nil = fail_on_nil == "true";
}


bool
Status::MessageDefinition::Observation::fresh() const
{
	return _observed_property.fresh();
}


bool
Status::MessageDefinition::Observation::test() const
{
	// Force fresh() to return false until next value change:
	_observed_property.read();

	if (_observed_property.is_nil())
		return _fail_on_nil;
	else
		return *_observed_property != _valid_state;
}


Status::MessageDefinition::MessageDefinition (QDomElement const& message_element)
{
	if (!message_element.hasAttribute ("message"))
		throw Xefis::Exception ("missing @message property on <message> element");

	QString severity_str = message_element.attribute ("severity");
	if (message_element.hasAttribute ("severity"))
	{
		if (severity_str == "critical")
			_severity = Severity::Critical;
		else if (severity_str == "warning")
			_severity = Severity::Warning;
		else
			throw Xefis::Exception ("invalid value for attribute @severity on <message> element - must be 'warning' or 'critical'");
	}

	for (QDomElement o: message_element)
		if (o == "observe")
			_observations.push_back (Observation (o));

	_message = message_element.attribute ("message");
}


Status::MessageDefinition::StateChange
Status::MessageDefinition::test()
{
	bool any_fresh = false;
	bool show = false;

	for (Observation const& o: _observations)
	{
		if (o.fresh())
		{
			any_fresh = true;
			show = o.test();
		}
	}

	if (any_fresh)
	{
		if (show == _shown)
			return NoChange;
		else
		{
			_shown = show;
			if (show)
				return Show;
			return Revoke;
		}
	}
	else
		return NoChange;
}


QColor
Status::MessageDefinition::color() const noexcept
{
	switch (_severity)
	{
		case Severity::Critical:	return Qt::red;
		case Severity::Warning:		return QColor (255, 200, 50);
	}
	return Qt::white;
}


Status::Status (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Instrument (module_manager, config)
{
	parse_settings (config, {
		{ "minimum-message-display-time", _minimum_display_time, false },
	});

	parse_properties (config, {
		{ "button.cursor-up", _button_cursor_up, false },
		{ "button.cursor-down", _button_cursor_down, false },
		{ "button.cursor-del", _button_cursor_del, false },
		{ "button.recall", _button_recall, false },
		{ "button.clear", _button_clear, false },
	});

	for (QDomElement e: config)
		if (e == "messages")
			for (QDomElement message_el: e)
				if (message_el == "message")
					_messages.push_back (MessageDefinition (message_el));

	_status_widget = new StatusWidget (this);
	_alert_command = new QProcess (this);
	QObject::connect (_alert_command, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (alert_finished (int, QProcess::ExitStatus)));

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_status_widget);
}


void
Status::data_updated()
{
	auto pressed = [](Xefis::PropertyBoolean& property) -> bool {
		return property.valid() && property.fresh() && *property;
	};

	if (pressed (_button_cursor_up))
		_status_widget->cursor_up();

	if (pressed (_button_cursor_down))
		_status_widget->cursor_down();

	if (pressed (_button_cursor_del))
		_status_widget->cursor_del();

	if (pressed (_button_recall))
		_status_widget->recall();

	if (pressed (_button_clear))
		if (Time::now() - _last_message_timestamp > _minimum_display_time)
			_status_widget->clear();

	bool sound_alert = false;
	for (auto& m: _messages)
	{
		switch (m.test())
		{
			case MessageDefinition::Show:
				sound_alert = true;
				// Hide old message:
				if (m.has_message_id())
				{
					_status_widget->remove_message (m.message_id());
					m.deassign_message_id();
				}
				// Show new one:
				m.set_message_id (_status_widget->add_message (m.message(), m.color()));
				_last_message_timestamp = Time::now();
				break;

			case MessageDefinition::Revoke:
				// Hide the message:
				if (m.has_message_id())
				{
					_status_widget->remove_message (m.message_id());
					m.deassign_message_id();
				}
				break;

			case MessageDefinition::NoChange:
				break;
		}
	}

	if (sound_alert)
		request_alert();
}


void
Status::alert_finished (int, QProcess::ExitStatus)
{
	_alert_started = false;

	if (_alert_requested)
	{
		_alert_requested = false;
		request_alert();
	}
}


void
Status::request_alert()
{
	if (_alert_started)
		_alert_requested = true;
	else
	{
		_alert_started = true;
		_alert_command->start ("aplay", { XEFIS_SHARED_DIRECTORY "/sounds/caution.wav" });
	}
}

