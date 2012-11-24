/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__INSTRUMENT_WIDGET_H__INCLUDED
#define XEFIS__CORE__INSTRUMENT_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QFont>
#include <QtGui/QColor>
#include <QtGui/QPen>
#include <QtGui/QWidget>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

class InstrumentWidget: public QWidget
{
  public:
	// Ctor
	InstrumentWidget (QWidget* parent);

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	/**
	 * Return min (width(), height());
	 */
	float
	wh() const;

	QPen
	get_pen (QColor const& color, float width);

	float
	pen_width (float scale = 1.0f) const;

	float
	font_size (float scale = 1.0f) const;

	int
	get_digit_width (QFont&) const;

	void
	update_fonts();

  protected:
	QFont				_font;
	QFont				_font_10_bold;
	QFont				_font_13_bold;
	QFont				_font_16_bold;
	QFont				_font_20_bold;
	float				_font_10_digit_width;
	float				_font_13_digit_width;
	float				_font_16_digit_width;
	float				_font_20_digit_width;
	float				_font_10_digit_height;
	float				_font_13_digit_height;
	float				_font_16_digit_height;
	float				_font_20_digit_height;
	QColor				_autopilot_color;
	QColor				_navigation_color;

	static const char	DIGITS[10];
	static const char*	MINUS_SIGN;
};


inline float
InstrumentWidget::wh() const
{
	return std::min (0.8f * width(), 1.f * height());
}


inline QPen
InstrumentWidget::get_pen (QColor const& color, float width)
{
	return QPen (color, pen_width (width), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}


inline float
InstrumentWidget::pen_width (float scale) const
{
	return std::max (0.f, scale * wh() / 325.f);
}


inline float
InstrumentWidget::font_size (float scale) const
{
	return std::max (1.f, scale * wh() / 375.f);
}

} // namespace Xefis

#endif

