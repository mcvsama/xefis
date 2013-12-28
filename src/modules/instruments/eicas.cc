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
#include "eicas.h"
#include "eicas_widget.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/eicas", EICAS);


EICAS::MessageDefinition::MessageDefinition (QDomElement const& message_element)
{
	if (!message_element.hasAttribute ("path"))
		throw Xefis::Exception ("missing @path property on <message> element");

	if (!message_element.hasAttribute ("message"))
		throw Xefis::Exception ("missing @message property on <message> element");

	QString fail_on = message_element.attribute ("fail-on");
	if (!message_element.hasAttribute ("fail-on"))
		throw Xefis::Exception ("missing @fail-on property on <message> element");
	if (fail_on != "true" && fail_on != "false")
		throw Xefis::Exception ("invalid value for attribute @fail-on on <message> element - must be 'true' or 'false'");

	QString fail_on_nil = message_element.attribute ("fail-on-nil");
	if (message_element.hasAttribute ("fail-on-nil") && fail_on_nil != "true" && fail_on_nil != "false")
		throw Xefis::Exception ("invalid value for attribute @fail-on-nil on <message> element - must be 'true' or 'false'");

	_observed_property.set_path (message_element.attribute ("path").toStdString());
	_valid_state = fail_on != "true";
	_fail_on_nil = fail_on_nil == "true";
	_message = message_element.attribute ("message");
}


EICAS::MessageDefinition::StateChange
EICAS::MessageDefinition::test() const
{
	if (_observed_property.fresh())
	{
		// Force fresh() to return false until next value change:
		_observed_property.read();

		if (_fail_on_nil && _observed_property.is_nil())
			return Show;

		if (_observed_property.valid())
		{
			if (*_observed_property == _valid_state)
				return Revoke;
			else
				return Show;
		}
	}

	return NoChange;
}


EICAS::EICAS (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Instrument (module_manager, config)
{
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

	_eicas_widget = new EICASWidget (this);
	_alert_command = new QProcess (this);
	QObject::connect (_alert_command, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (alert_finished (int, QProcess::ExitStatus)));

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_eicas_widget);
}


void
EICAS::data_updated()
{
	if (_button_cursor_up.valid() && _button_cursor_up.fresh() && *_button_cursor_up)
		_eicas_widget->cursor_up();

	if (_button_cursor_down.valid() && _button_cursor_down.fresh() && *_button_cursor_down)
		_eicas_widget->cursor_down();

	if (_button_cursor_del.valid() && _button_cursor_del.fresh() && *_button_cursor_del)
		_eicas_widget->cursor_del();

	if (_button_recall.valid() && _button_recall.fresh() && *_button_recall)
		_eicas_widget->recall();

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
					_eicas_widget->remove_message (m.message_id());
					m.deassign_message_id();
				}
				// Show new one:
				m.set_message_id (_eicas_widget->add_message (m.message()));
				break;

			case MessageDefinition::Revoke:
				// Hide the message:
				if (m.has_message_id())
				{
					_eicas_widget->remove_message (m.message_id());
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
EICAS::alert_finished (int, QProcess::ExitStatus)
{
	_alert_started = false;

	if (_alert_requested)
	{
		_alert_requested = false;
		request_alert();
	}
}


void
EICAS::request_alert()
{
	if (_alert_started)
		_alert_requested = true;
	else
	{
		_alert_started = true;
		_alert_command->start ("aplay", { XEFIS_SHARED_DIRECTORY "/sounds/caution.wav" });
	}
}

