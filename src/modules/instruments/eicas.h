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

#ifndef XEFIS__MODULES__INSTRUMENTS__EICAS_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__EICAS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QProcess>
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/instrument.h>


class EICASWidget;

class EICAS: public Xefis::Instrument
{
	Q_OBJECT

	/**
	 * Configuration for a single message
	 * shown when an observed property changes
	 * state.
	 */
	class MessageDefinition
	{
	  public:
		/**
		 * Severity affects color of the message
		 * and resulting alert sound. (TODO)
		 */
		enum Severity
		{
			Critical,
			Warning,
		};

		/**
		 * Result returned by test().
		 */
		enum StateChange
		{
			Show,
			Revoke,
			NoChange,
		};

	  public:
		// Ctor
		explicit MessageDefinition (QDomElement const& message_element);

		/**
		 * Return severity of the message.
		 */
		Severity
		severity() const noexcept;

		/**
		 * Test whether message should be shown or not,
		 * according to the configuration and current property state.
		 */
		StateChange
		test() const;

		/**
		 * Message to show on EICAS.
		 */
		QString
		message() const noexcept;

		/**
		 * Set message ID from EICASWidget.
		 */
		void
		set_message_id (uint64_t id) noexcept;

		/**
		 * Deassigns message ID from EICASWidget.
		 */
		void
		deassign_message_id() noexcept;

		/**
		 * Return associated message ID.
		 */
		uint64_t
		message_id() const noexcept;

		/**
		 * Return true if message ID from EICASWidget has been assigned.
		 */
		bool
		has_message_id() const noexcept;

		/**
		 * Return color appropriate for this message.
		 */
		QColor
		color() const noexcept;

	  private:
		Xefis::PropertyBoolean	_observed_property;
		bool					_valid_state		= true;
		bool					_fail_on_nil		= false;
		Severity				_severity			= Severity::Warning;
		QString					_message;
		uint64_t				_message_id			= 0;
		bool					_has_message		= false;
	};

  public:
	// Ctor
	EICAS (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private slots:
	/**
	 * Called when alert command finishes.
	 */
	void
	alert_finished (int exit_code, QProcess::ExitStatus exit_status);

  private:
	/**
	 * Request alert sound. If it's sounding already,
	 * set _restart_alert to true and start a timer
	 * to restart it again soon.
	 */
	void
	request_alert();

  private:
	EICASWidget*					_eicas_widget		= nullptr;
	Xefis::PropertyBoolean			_button_cursor_up;
	Xefis::PropertyBoolean			_button_cursor_down;
	Xefis::PropertyBoolean			_button_cursor_del;
	Xefis::PropertyBoolean			_button_recall;
	Xefis::PropertyBoolean			_button_clear;
	std::list<MessageDefinition>	_messages;
	bool							_alert_requested	= false;
	bool							_alert_started		= false;
	QProcess*						_alert_command;
};


inline EICAS::MessageDefinition::Severity
EICAS::MessageDefinition::severity() const noexcept
{
	return _severity;
}


inline QString
EICAS::MessageDefinition::message() const noexcept
{
	return _message;
}


inline void
EICAS::MessageDefinition::set_message_id (uint64_t id) noexcept
{
	_message_id = id;
	_has_message = true;
}


inline void
EICAS::MessageDefinition::deassign_message_id() noexcept
{
	_message_id = 0;
	_has_message = false;
}


inline uint64_t
EICAS::MessageDefinition::message_id() const noexcept
{
	return _message_id;
}


inline bool
EICAS::MessageDefinition::has_message_id() const noexcept
{
	return _has_message;
}

#endif

