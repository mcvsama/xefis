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

// Qt:
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module_manager.h>
#include <xefis/core/stdexcept.h>
#include <xefis/core/xefis.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/qdom_iterator.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "status.h"
#include "status_widget.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/status", Status)


Status::MessageDefinition::Observation::Observation (QDomElement const& observe_element)
{
	QString fail_on_nil = observe_element.attribute ("fail-on-nil");
	if (observe_element.hasAttribute ("fail-on-nil") && fail_on_nil != "true" && fail_on_nil != "false")
		throw xf::BadDomAttribute (observe_element, "fail-on-nil", "must be 'true' or 'false'");

	if (!observe_element.hasAttribute ("path"))
		throw xf::MissingDomAttribute (observe_element, "path");

	QString fail_on = observe_element.attribute ("fail-on");
	if (!observe_element.hasAttribute ("fail-on"))
		throw xf::MissingDomAttribute (observe_element, "fail-on");
	if (fail_on != "true" && fail_on != "false")
		throw xf::BadDomAttribute (observe_element, "fail-on", "must be 'true' or 'false'");

	_observed_property.set_path (xf::PropertyPath (observe_element.attribute ("path")));
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
		throw xf::MissingDomAttribute (message_element, "message");

	QString severity_str = message_element.attribute ("severity");
	if (message_element.hasAttribute ("severity"))
	{
		if (severity_str == "caution")
			_severity = Severity::Caution;
		else if (severity_str == "warning")
			_severity = Severity::Warning;
		else
			throw xf::BadDomAttribute (message_element, "severity", "must be 'caution' or 'warning'");
	}

	for (QDomElement const& o: xf::iterate_sub_elements (message_element))
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
		case Severity::Caution:
			return QColor (255, 200, 50);

		case Severity::Warning:
			return Qt::red;
	}
	return Qt::white;
}


Status::Status (v1::ModuleManager* module_manager, QDomElement const& config):
	Instrument (module_manager, config)
{
	parse_settings (config, {
		{ "minimum-message-display-time", _minimum_display_time, false },
	});

	parse_properties (config, {
		{ "input.cursor", _input_cursor_value, false },
		{ "input.button.cursor-del", _input_button_cursor_del, false },
		{ "input.button.recall", _input_button_recall, false },
		{ "input.button.clear", _input_button_clear, false },
		{ "input.button.master-caution", _input_button_master_caution, false },
		{ "input.button.master-warning", _input_button_master_warning, false },
		{ "output.master-caution", _output_master_caution, false },
		{ "output.master-warning", _output_master_warning, false },
	});

	for (QDomElement const& e: xf::iterate_sub_elements (config))
		if (e == "messages")
			for (QDomElement const& message_el: xf::iterate_sub_elements (e))
				if (message_el == "message")
					_messages.push_back (MessageDefinition (message_el));

	_status_widget = new StatusWidget (this);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_status_widget);

	_input_cursor_decoder = std::make_unique<xf::DeltaDecoder> (_input_cursor_value, [this](int delta) {
		if (delta > 0)
		{
			for (int i = 0; i < delta; ++i)
				_status_widget->cursor_up();
		}
		else if (delta < 0)
		{
			for (int i = 0; i > delta; --i)
				_status_widget->cursor_down();
		}
	});

	_input_cursor_decoder->call (0);
}


void
Status::data_updated()
{
	_input_cursor_decoder->data_updated();

	auto pressed = [](v1::PropertyBoolean& property) -> bool {
		return property.valid_and_fresh() && *property;
	};

	if (pressed (_input_button_master_caution))
		_output_master_caution = false;

	if (pressed (_input_button_master_warning))
		_output_master_warning = false;

	if (pressed (_input_button_cursor_del))
		_status_widget->cursor_del();

	if (pressed (_input_button_recall))
		_status_widget->recall();

	if (pressed (_input_button_clear))
		if (xf::TimeHelper::now() - _last_message_timestamp > _minimum_display_time)
			_status_widget->clear();

	for (auto& m: _messages)
	{
		switch (m.test())
		{
			case MessageDefinition::Show:
				// Hide old message:
				if (m.has_message_id())
				{
					_status_widget->remove_message (m.message_id());
					m.deassign_message_id();
				}
				// Show new one:
				m.set_message_id (_status_widget->add_message (m.message(), m.color()));
				_last_message_timestamp = xf::TimeHelper::now();
				// Master* buttons?
				switch (m.severity())
				{
					case MessageDefinition::Severity::Caution:
						_output_master_warning = true;
						break;

					case MessageDefinition::Severity::Warning:
						_output_master_warning = true;
						break;
				}
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
}

