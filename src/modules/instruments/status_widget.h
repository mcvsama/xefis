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

#ifndef XEFIS__MODULES__INSTRUMENTS__STATUS_WIDGET_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__STATUS_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtCore/QTimer>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument_widget.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/utility/painter.h>


class StatusWidget:
	public Xefis::InstrumentWidget,
	public Xefis::InstrumentAids
{
	Q_OBJECT

	static constexpr Time MessageHideTimeout = 5_s;

  public:
	class Message
	{
	  public:
		/**
		 * Mark message as outdated (condition for displaying the message is false).
		 * Message marked as such is dispayed in different color.
		 */
		void
		mark_as_outdated();

	  public:
		uint64_t	id;
		QString		message;
		bool		outdated;
		QColor		color;
	};

  private:
	typedef std::vector<Message> Messages;

  public:
	// Ctor
	StatusWidget (QWidget* parent);

	/**
	 * Add new message to show.
	 */
	uint64_t
	add_message (QString const& message, QColor color);

	/**
	 * Remove message identified by ID.
	 * Doesn't hide the message immediately, instead schedule it to be removed
	 * later with do_remove_message() method.
	 */
	void
	remove_message (uint64_t id);

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

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

  private:
	/**
	 * Calculate sizes, viewports, etc.
	 */
	void
	recompute();

	/**
	 * Compute scroll value needed to display messages.
	 */
	void
	solve_scroll_and_cursor();

	/**
	 * Removes message identified by ID.
	 */
	void
	do_remove_message (uint64_t id);

	/**
	 * Find message by ID, either in hidden or shown message vectors.
	 * \param	vector_ptr If not null, it is set to the vector in which the message was found.
	 * \param	iterator_ptr If not null, it is set to the iterator that points to the message.
	 * \return	Message pointer or nullptr, if not found.
	 */
	Message*
	find_message (uint64_t id, Messages** vector_ptr = nullptr, Messages::iterator** iterator_ptr = nullptr);

  private:
	double						_line_height		= 0.0;
	double						_arrow_height		= 0.0;
	int							_scroll				= 0;
	int							_cursor				= 0;
	int							_max_shown_messages	= 0;
	uint64_t					_id_generator		= 0;
	bool						_blink_status		= false;
	bool						_cursor_visible		= false;
	QFont						_font;
	QRectF						_viewport;
	Messages					_shown_messages;
	Messages					_hidden_messages;
	Unique<QTimer>				_blinking_timer;
	Unique<QTimer>				_cursor_hide_timer;
};

#endif

