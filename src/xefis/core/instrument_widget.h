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

#ifndef XEFIS__CORE__INSTRUMENT_WIDGET_H__INCLUDED
#define XEFIS__CORE__INSTRUMENT_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QFont>
#include <QtGui/QColor>
#include <QtGui/QPen>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

class InstrumentWidget: public QWidget
{
  public:
	// Ctor
	InstrumentWidget (QWidget* parent, float height_fot_width, float master_pen_scale, float master_font_scale);

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	/**
	 * Return lesser dimension corrected by height-for-width.
	 * Useful for computing feature sizes, but for line widths
	 * you should rather use win_wh() or pen_width().
	 */
	float
	wh() const;

	/**
	 * Return lesser dimension of the top-level window.
	 */
	float
	win_wh() const;

	QPen
	get_pen (QColor const& color, float width);

	float
	pen_width (float scale = 1.0f) const;

	float
	font_size (float scale = 1.0f) const;

	int
	get_digit_width (QFont&) const;

	float
	translate_descent (QFontMetricsF& metrics_1, QFontMetricsF& metrics_2);

	void
	update_sizes();

	void
	centrify (QRectF& rectf) const;

	Angle
	arc_degs (Angle deg) const;

	Angle
	arc_span (Angle deg) const;

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
	float				_height_for_width		= 1.f;
	float				_master_pen_scale		= 1.f;
	float				_master_font_scale		= 1.f;
	QPen				_autopilot_pen_1;
	QPen				_autopilot_pen_2;
	float				_w;
	float				_h;
	float				_window_w;
	float				_window_h;

	static const char	DIGITS[10];
	static const char*	MINUS_SIGN;
};


inline float
InstrumentWidget::wh() const
{
	return std::min (_height_for_width * _w, _h);
}


inline float
InstrumentWidget::win_wh() const
{
	return std::min (_window_w, _window_h);
}


inline QPen
InstrumentWidget::get_pen (QColor const& color, float width)
{
	return QPen (color, pen_width (width), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}


inline float
InstrumentWidget::pen_width (float scale) const
{
	return std::max (0.f, _master_pen_scale * scale * win_wh() / 460.f);
}


inline float
InstrumentWidget::font_size (float scale) const
{
	return std::max (1.f, _master_font_scale * scale * win_wh() / 610.f);
}


inline void
InstrumentWidget::centrify (QRectF& rectf) const
{
	rectf.translate (-0.5f * rectf.width(), -0.5f * rectf.height());
}


inline Angle
InstrumentWidget::arc_degs (Angle deg) const
{
	return -16.f * (deg - 90_deg);
}


inline Angle
InstrumentWidget::arc_span (Angle deg) const
{
	return -16.f * deg;
}

} // namespace Xefis

#endif

