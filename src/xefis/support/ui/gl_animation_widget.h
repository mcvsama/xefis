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

#ifndef XEFIS__SUPPORT__UI__GL_ANIMATION_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__UI__GL_ANIMATION_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QApplication>
#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QOpenGLWidget>
#include <QPainter>
#include <QTimer>

// Standard:
#include <cstddef>
#include <variant>


namespace xf {

/**
 * Calls specified painting function in a loop with specified frequency.
 * The painted images are animated in a widget.
 */
class GLAnimationWidget: public QOpenGLWidget
{
  public:
	enum FPSMode
	{
		// Auto update FPS according to screen settings on which the widget is placed:
		AutoFPS,
	};

	using RefreshRate = std::variant<si::Frequency, FPSMode>;

  public:
	// Ctor
	explicit
	GLAnimationWidget (QWidget* parent, RefreshRate, std::function<void (QOpenGLPaintDevice&)> display_function);

	/**
	 * Return current FPS (frames per second).
	 */
	[[nodiscard]]
	si::Frequency
	refresh_rate() const noexcept
		{ return _current_refresh_rate; }

	/**
	 * Set FPS (frames per second) aka refresh rate.
	 */
	void
	set_refresh_rate (RefreshRate);

	// API of QOpenGLWidget
	void
	paintGL();

  private:
	void
	paint();

	void
	update_refresh_rate();

  private:
	RefreshRate									_requested_refresh_rate;
	si::Frequency								_current_refresh_rate;
	std::unique_ptr<QOpenGLPaintDevice>			_open_gl_device;
	std::function<void (QOpenGLPaintDevice&)>	_display_function;
	QTimer*										_refresh_timer;
};

} // namespace xf

#endif

