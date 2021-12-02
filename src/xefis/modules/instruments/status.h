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
#include <functional>
#include <optional>

// Qt:
#include <QtCore/QTimer>

// Neutrino:
#include <neutrino/synchronized.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/instrument.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/instrument/instrument_support.h>
#include <xefis/support/sockets/socket_button.h>
#include <xefis/support/sockets/socket_delta_decoder.h>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class StatusIO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Time>	status_minimum_display_time	{ this, "status_minimum_display_time", 5_s };

	/*
	 * Input
	 */

	xf::ModuleIn<int64_t>	cursor_value			{ this, "cursor/value" };
	xf::ModuleIn<bool>		button_cursor_del		{ this, "button/cursor-del" };
	xf::ModuleIn<bool>		button_recall			{ this, "button/recall" };
	xf::ModuleIn<bool>		button_clear			{ this, "button/clear" };
	xf::ModuleIn<bool>		button_master_caution	{ this, "button/master-caution" };
	xf::ModuleIn<bool>		button_master_warning	{ this, "button/master-warning" };

	/*
	 * Output
	 */

	xf::ModuleOut<bool>		master_caution			{ this, "master-caution" };
	xf::ModuleOut<bool>		master_warning			{ this, "master-warning" };
};


/**
 * Widget showing status messages.
 */
class Status:
	public xf::Instrument<StatusIO>,
	private xf::InstrumentSupport
{
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

	static constexpr si::Time kMessageHideTimeout = 5_s;

	class Message
	{
	  public:
		// Ctor
		explicit
		Message (std::string_view const& text, Severity);

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
		 * Show message when given socket has given value.
		 *
		 * \return	pointer to itself.
		 */
		template<class Value>
			Message*
			show_when (xf::Socket<Value> const&, Value);

		/**
		 * Show message when given socket has the nil value.
		 *
		 * \return	pointer to itself.
		 */
		Message*
		show_when (xf::BasicSocket const&, xf::Nil);

		/**
		 * Show message when given function returns true.
		 *
		 * \return	pointer to itself.
		 */
		Message*
		show_when (std::function<bool()>);

		/**
		 * Process observed socket and compute whether message should be visible or not. Call this method once every
		 * xf::Cycle.
		 */
		void
		process (si::Time now);

		/**
		 * Return true when message should be visible on screen
		 * according to the configuration and current socket state.
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

  private:
	class Cache
	{
	  public:
		QFont	font;
		float	line_height				{ 0.0 };
		float	arrow_height			{ 0.0 };
		QRectF	viewport;
		int		max_visible_messages	{ 0 };
		int		scroll_pos				{ 0 };
		int		cursor_pos				{ 0 };
		bool	cursor_visible			{ false };

	  public:
		/**
		 * Compute scroll value needed to display messages.
		 */
		void
		solve_scroll_and_cursor (std::vector<Message*> const& visible_messages);
	};

	struct PaintingParams
	{
		std::vector<Message*>	visible_messages;
	};

  public:
	// Ctor
	explicit
	Status (std::unique_ptr<StatusIO>, xf::Graphics const&, std::string_view const& instance = {});

	/**
	 * Configure new message.
	 * Use returned pointer to operate on the Message object and add observers.
	 */
	Message&
	add_message (std::string_view const& text, Severity);

	// Module API
	void
	process (xf::Cycle const&) override;

	// Instrument API
	std::packaged_task<void()>
	paint (xf::PaintRequest) const override;

  private:
	void
	async_paint (xf::PaintRequest const&, PaintingParams const&) const;

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
	 * Hide all shown messages except for those which aren't displayed for at least minimum display time.
	 */
	void
	clear();

  private:
	xf::SocketButton							_button_cursor_del		{ io.button_cursor_del, [this]{ cursor_del(); } };
	xf::SocketButton							_button_recall			{ io.button_recall, [this]{ recall(); } };
	xf::SocketButton							_button_clear			{ io.button_clear, [this]{ clear(); } };
	xf::SocketButton							_button_master_caution	{ io.button_master_caution, [this]{ io.master_caution = false; } };
	xf::SocketButton							_button_master_warning	{ io.button_master_warning, [this]{ io.master_warning = false; } };
	std::unique_ptr<xf::SocketDeltaDecoder<>>	_input_cursor_decoder;
	std::vector<Message>						_messages;
	std::vector<Message*>						_hidden_messages;
	std::vector<Message*>						_visible_messages;
	si::Time									_last_message_timestamp;
	std::unique_ptr<QTimer>						_blink_timer;
	std::unique_ptr<QTimer>						_cursor_hide_timer;
	std::atomic<bool>							_blink_show						{ false };
	xf::Synchronized<Cache> mutable				_cache;
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
	Status::Message::show_when (xf::Socket<Value> const& socket, Value value)
	{
		_conditions.push_back ([&socket,value] {
			return socket && *socket == value;
		});
		return this;
	}


inline Status::Message*
Status::Message::show_when (xf::BasicSocket const& socket, xf::Nil)
{
	_conditions.push_back ([&socket] {
		return socket == xf::nil;
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

