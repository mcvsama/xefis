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

#ifndef XEFIS__SUPPORT__UI__WIDGET_AIDS_H__INCLUDED
#define XEFIS__SUPPORT__UI__WIDGET_AIDS_H__INCLUDED

// Standard:
#include <cstddef>
#include <memory>

// Qt:
#include <QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qutils.h>


namespace xf {

class Widget: public QWidget
{
  public:
	// Ctor
	explicit
	Widget (QWidget* parent = nullptr, Qt::WindowFlags = Qt::Widget);

	/**
	 * Return current PixelDensity.
	 */
	si::PixelDensity
	pixel_density() const;

	/**
	 * Return number of pixels that correspond to the given real length on the screen.
	 */
	float
	pixels (si::Length) const;

	/**
	 * Return number of pixels that correspond to the given line-heights of text.
	 */
	float
	em_pixels (float ems = 1.0f) const;

  public:
	/**
	 * Return simple uniform-color widget.
	 */
	static QWidget*
	create_color_widget (QColor, QWidget* parent);

	/**
	 * Return label with colored strip.
	 * \param	strip_position
	 *			Qt::AlignTop, Qt::AlignBottom, Qt::AlignLeft or Qt::AlignRight.
	 */
	QWidget*
	create_colored_strip_label (QString const& label, QColor color, Qt::Alignment strip_position, QWidget* parent) const;
};


inline si::PixelDensity
Widget::pixel_density() const
{
	return si::PixelDensity (this->logicalDpiY());
}


inline float
Widget::pixels (si::Length width) const
{
	return xf::pixels (width, pixel_density());
}


inline float
Widget::em_pixels (float ems) const
{
	int v = font().pixelSize();

	if (v != -1)
		return ems * v;
	else
		return ems * font().pointSize() * xf::pixels_per_point (pixel_density());
}

} // namespace xf

#endif

