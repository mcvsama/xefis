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

#ifndef XEFIS__MODULES__INSTRUMENTS__STATUS_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__STATUS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/property.h>
#include <xefis/core/instrument.h>
#include <xefis/utility/delta_decoder.h>


class StatusWidget;

class Status: public xf::Instrument
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
		 * and resulting alert sound.
		 */
		enum Severity
		{
			Caution,
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

		/**
		 * Observed property and conditions.
		 */
		class Observation
		{
		  public:
			// Ctor
			explicit
			Observation (QDomElement const& observe_element);

			/**
			 * Return true, if property has changed its value since
			 * last call to the test() method.
			 */
			bool
			fresh() const;

			/**
			 * Return true, if conditions for showing message apply.
			 */
			bool
			test() const;

		  private:
			xf::PropertyBoolean	_observed_property;
			bool					_valid_state	= true;
			bool					_fail_on_nil	= false;
		};

	  public:
		// Ctor
		explicit
		MessageDefinition (QDomElement const& message_element);

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
		test();

		/**
		 * Message to show on Status.
		 */
		QString
		message() const noexcept;

		/**
		 * Set message ID from StatusWidget.
		 */
		void
		set_message_id (uint64_t id) noexcept;

		/**
		 * Deassigns message ID from StatusWidget.
		 */
		void
		deassign_message_id() noexcept;

		/**
		 * Return associated message ID.
		 */
		uint64_t
		message_id() const noexcept;

		/**
		 * Return true if message ID from StatusWidget has been assigned.
		 */
		bool
		has_message_id() const noexcept;

		/**
		 * Return color appropriate for this message.
		 */
		QColor
		color() const noexcept;

	  private:
		std::vector<Observation>	_observations;
		bool						_shown				= false;
		Severity					_severity			= Severity::Warning;
		QString						_message;
		uint64_t					_message_id			= 0;
		bool						_has_message		= false;
	};

  public:
	// Ctor
	Status (xf::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	StatusWidget*					_status_widget			= nullptr;
	xf::PropertyInteger				_input_cursor_value;
	Unique<xf::DeltaDecoder>		_input_cursor_decoder;
	xf::PropertyBoolean				_input_button_cursor_del;
	xf::PropertyBoolean				_input_button_recall;
	xf::PropertyBoolean				_input_button_clear;
	xf::PropertyBoolean				_input_button_master_caution;
	xf::PropertyBoolean				_input_button_master_warning;
	xf::PropertyBoolean				_output_master_caution;
	xf::PropertyBoolean				_output_master_warning;
	std::list<MessageDefinition>	_messages;
	Time							_minimum_display_time	= 5_s;
	Time							_last_message_timestamp;
};


inline Status::MessageDefinition::Severity
Status::MessageDefinition::severity() const noexcept
{
	return _severity;
}


inline QString
Status::MessageDefinition::message() const noexcept
{
	return _message;
}


inline void
Status::MessageDefinition::set_message_id (uint64_t id) noexcept
{
	_message_id = id;
	_has_message = true;
}


inline void
Status::MessageDefinition::deassign_message_id() noexcept
{
	_message_id = 0;
	_has_message = false;
}


inline uint64_t
Status::MessageDefinition::message_id() const noexcept
{
	return _message_id;
}


inline bool
Status::MessageDefinition::has_message_id() const noexcept
{
	return _has_message;
}

#endif

