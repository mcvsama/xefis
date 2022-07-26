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

#ifndef XEFIS__SUPPORT__UI__CANVAS_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__UI__CANVAS_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/qt/qutils.h>

// Qt:
#include <QWidget>

// Standard:
#include <cstddef>
#include <memory>


namespace xf {

class CanvasWidget: public QWidget
{
  public:
	// Ctor
	explicit
	CanvasWidget (QWidget* parent = nullptr, Qt::WindowFlags = Qt::Widget);

	// QWidget API
	void
	resizeEvent (QResizeEvent*) override;

	// QWidget API
	void
	paintEvent (QPaintEvent*) override;

	// QWidget API
	void
	changeEvent (QEvent*) override;

  protected:
	/**
	 * This method should repaint the canvas when called.
	 */
	virtual void
	update_canvas() = 0;

	/**
	 * Get the canvas to paint on.
	 */
	[[nodiscard]]
	QImage&
	canvas();

	/**
	 * Mark object as need-to-be-repainted.
	 */
	void
	mark_dirty()
		{ _dirty = true; }

  private:
	void
	ensure_canvas_exists();

  private:
	std::optional<QImage>	_canvas;
	bool					_dirty		{ false };
};

} // namespace xf

#endif

