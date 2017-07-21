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
#include <optional>

// Qt:
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/instrument.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/utility/v2/actions.h>
#include <xefis/utility/v2/delta_decoder.h>


class StatusWidget;

/**
 * Widget showing status messages.
 */
class Status:
	public v2::Instrument,
	protected xf::InstrumentAids
{
	Q_OBJECT

  public:
	/*
	 * Settings
	 */

	v2::Setting<si::Time>	minimum_display_time			{ this, 5_s };

	/*
	 * Input
	 */

	v2::PropertyIn<int64_t>	input_cursor_value				{ this, "/input/cursor/value" };
	v2::PropertyIn<bool>	input_button_cursor_del			{ this, "/input/button/cursor-del" };
	v2::PropertyIn<bool>	input_button_recall				{ this, "/input/button/recall" };
	v2::PropertyIn<bool>	input_button_clear				{ this, "/input/button/clear" };
	v2::PropertyIn<bool>	input_button_master_caution		{ this, "/input/button/master-caution" };
	v2::PropertyIn<bool>	input_button_master_warning		{ this, "/input/button/master-warning" };

	/*
	 * Output
	 */

	v2::PropertyOut<bool>	output_master_caution			{ this, "/output/master-caution" };
	v2::PropertyOut<bool>	output_master_warning			{ this, "/output/master-warning" };

  public:
	/**
	 * Severity affects color of the message and resulting alert sound.
	 */
	enum Severity
	{
		Notice,
		Caution,
		Warning,
	};

	static constexpr Time kMessageHideTimeout = 5_s;

	class Message
	{
	  public:
		// Ctor
		explicit
		Message (std::string const& text, Severity);

		/**
		 * Message to show on Status.
		 */
		std::string const&
		text() const noexcept;

		/**
		 * Return severity of the message.
		 */
		Severity
		severity() const noexcept;

		/**
		 * Show message when given property has given value.
		 *
		 * \return	pointer to itself.
		 */
		template<class Value>
			Message*
			show_when (v2::Property<Value> const&, Value);

		/**
		 * Show message when given property has the nil value.
		 *
		 * \return	pointer to itself.
		 */
		Message*
		show_when (v2::BasicProperty const&, v2::Nil);

		/**
		 * Show message when given function returns true.
		 *
		 * \return	pointer to itself.
		 */
		Message*
		show_when (std::function<bool()>);

		/**
		 * Process observed property and compute whether message should be visible or not. Call this method once every
		 * xf::Cycle.
		 */
		void
		process (si::Time now);

		/**
		 * Return true when message should be visible on screen
		 * according to the configuration and current property state.
		 */
		bool
		should_be_shown() const noexcept;

		/**
		 * Return color appropriate for this message.
		 */
		QColor
		color() const noexcept;

		/**
		 * Return true if message is outdated, that is condition for displaying the message has became false.
		 * Message marked as such is dispayed in different color.
		 */
		bool
		outdated (si::Time now) const noexcept;

	  private:
		std::string							_text;
		Severity							_severity;
		std::vector<std::function<bool()>>	_conditions;
		// True if any of conditions added with show_when() is true:
		bool								_condition_up;
		bool								_should_be_shown;
		std::optional<si::Time>				_outdated;
		bool								_deleted;
	};

  public:
	// Ctor
	explicit
	Status (std::string const& instance = {});

	/**
	 * Configure new message.
	 * Use returned pointer to operate on the Message object and add observers.
	 */
	Message*
	add_message (std::string const& text, Severity);

	// Module API
	void
	process (v2::Cycle const&) override;

  protected:
	// QWidget API
	void
	resizeEvent (QResizeEvent*) override;

	// QWidget API
	void
	paintEvent (QPaintEvent*) override;

  private:
	/**
	 * Move cursor up.
	 */
	void
	cursor_up();

	/**
	 * Move cursor down.
	 */
	void
	cursor_down();

	/**
	 * Hide message highlighted under cursor, if cursor is visible.
	 */
	void
	cursor_del();

	/**
	 * Recall (show) all hidden messages.
	 */
	void
	recall();

	/**
	 * Hide all shown messages.
	 */
	void
	clear();

	/**
	 * Calculate sizes, viewports, etc.
	 */
	void
	recompute_widget();

	/**
	 * Compute scroll value needed to display messages.
	 */
	void
	solve_scroll_and_cursor();

  private:
	v2::PropChangedTo<bool>				_button_master_caution_pressed { input_button_master_caution, true };
	v2::PropChangedTo<bool>				_button_master_warning_pressed { input_button_master_warning, true };
	v2::PropChangedTo<bool>				_button_cursor_del_pressed { input_button_cursor_del, true };
	v2::PropChangedTo<bool>				_button_recall_pressed { input_button_recall, true };
	v2::PropChangedTo<bool>				_button_clear_pressed { input_button_clear, true };
	std::unique_ptr<v2::DeltaDecoder>	_input_cursor_decoder;
	std::vector<Message>				_messages;
	std::vector<Message*>				_hidden_messages;
	std::vector<Message*>				_visible_messages;
	si::Time							_last_message_timestamp;
	Unique<QTimer>						_blink_timer;
	Unique<QTimer>						_cursor_hide_timer;
	double								_line_height			{ 0.0 };
	double								_arrow_height			{ 0.0 };
	int									_scroll_pos				{ 0 };
	int									_cursor_pos				{ 0 };
	int									_max_visible_messages	{ 0 };
	bool								_blink_show				{ false };
	bool								_cursor_visible			{ false };
	QFont								_font;
	QRectF								_viewport;
};


inline std::string const&
Status::Message::text() const noexcept
{
	return _text;
}


inline Status::Severity
Status::Message::severity() const noexcept
{
	return _severity;
}


template<class Value>
	inline Status::Message*
	Status::Message::show_when (v2::Property<Value> const& property, Value value)
	{
		_conditions.push_back ([&property,value] {
			return property && *property == value;
		});
		return this;
	}


inline Status::Message*
Status::Message::show_when (v2::BasicProperty const& property, v2::Nil)
{
	_conditions.push_back ([&property] {
		return property == v2::nil;
	});
	return this;
}


inline Status::Message*
Status::Message::show_when (std::function<bool()> predicate)
{
	_conditions.push_back (predicate);
	return this;
}


inline bool
Status::Message::should_be_shown() const noexcept
{
	return _should_be_shown;
}


inline bool
Status::Message::outdated (si::Time now) const noexcept
{
	return _outdated && *_outdated + kMessageHideTimeout < now;
}

#endif

