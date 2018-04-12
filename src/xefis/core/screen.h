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

#ifndef XEFIS__CORE__SCREEN_H__INCLUDED
#define XEFIS__CORE__SCREEN_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QSize>
#include <QImage>
#include <QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/instrument.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/registration_proof.h>
#include <xefis/utility/registry.h>


namespace xf {

namespace detail {

/**
 * Additional information for each instrument needed by the Screen object,
 * such as its position on the screen.
 */
class Details
{
  public:
	QRect	rect;
	QImage	canvas;
};

} // namespace detail


class Screen:
	public QWidget,
	public Registry<BasicInstrument, detail::Details>,
	private Noncopyable
{
	Q_OBJECT

  public:
	// Ctor
	explicit
	Screen (QRect, si::Frequency refresh_rate);

	/**
	 * Set position and size of an instrument.
	 */
	void
	set (BasicInstrument const&, QRect);

  protected:
	// QWidget API:
	void
	paintEvent (QPaintEvent*) override;

	// QWidget API:
	void
	resizeEvent (QResizeEvent*) override;

  private:
	/**
	 * Request painting of all instruments on the canvas-buffer.
	 */
	void
	paint_instruments_to_buffer();

	/**
	 * Prepare canvas for an instrument.
	 * Ensure it has requested size and set it to full alpha with color black.
	 */
	void
	prepare_canvas_for_instrument (QImage&, QSize);

  private slots:
	/**
	 * Called when next frame should be painted.
	 */
	void
	refresh();

  private:
	QTimer*	_refresh_timer;
	QImage	_canvas;
};

} // namespace xf

#endif

