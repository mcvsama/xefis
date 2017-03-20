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

#ifndef XEFIS__CORE__INSTRUMENT_AIDS_H__INCLUDED
#define XEFIS__CORE__INSTRUMENT_AIDS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QFont>
#include <QtGui/QColor>
#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/painter.h>


namespace xf {

class InstrumentAids
{
  public:
	/**
	 * RAII-style painting token. It calls begin() and end() methods,
	 * when constructed and destructed (last copy), respectively.
	 */
	class Token
	{
	  public:
		// Ctor
		explicit
		Token (QPainter*, QPaintDevice*);

		// Dtor
		~Token();

	  private:
		QPainter* _painter;
	};

  public:
	// Ctor
	explicit
	InstrumentAids (float height_for_width);

	/**
	 * Set master pen and font scaling.
	 */
	void
	set_scaling (float pen_scale, float font_scale);

  public:
	/**
	 * Return xf::Painter to use. Use begin() and end() methods to begin/end painting.
	 */
	Painter&
	painter() noexcept;

	/**
	 * Get RAII-style painting token. You must create token with this method,
	 * before attemtping to paint. You can paint as long as the token
	 * lives (or the last copy lives).
	 */
	Shared<Token>
	get_token (QPaintDevice* device);

	/**
	 * Clear the widget with black.
	 */
	void
	clear_background (QColor = Qt::black);

	/**
	 * Needs to be called when instrument widget size changes.
	 */
	void
	update_sizes (QSize const& size, QSize const& window_size);

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
	get_pen (QColor const& color, float width, Qt::PenStyle style = Qt::SolidLine, Qt::PenCapStyle cap = Qt::RoundCap, Qt::PenJoinStyle join = Qt::MiterJoin) const;

	float
	pen_width (float scale = 1.0f) const;

	float
	font_size (float scale = 1.0f) const;

	static int
	get_digit_width (QFont&);

	static void
	centrify (QRectF& rectf);

	static float
	arc_degs (Angle deg);

	static float
	arc_span (Angle deg);

  public:
	xf::Painter				_painter;
	xf::TextPainter::Cache	_text_painter_cache;
	QFont					_font;
	QFont					_font_8;
	QFont					_font_10;
	QFont					_font_13;
	QFont					_font_16;
	QFont					_font_18;
	QFont					_font_20;
	float					_font_8_digit_width		= 0.f;
	float					_font_10_digit_width	= 0.f;
	float					_font_13_digit_width	= 0.f;
	float					_font_16_digit_width	= 0.f;
	float					_font_18_digit_width	= 0.f;
	float					_font_20_digit_width	= 0.f;
	float					_font_8_digit_height	= 0.f;
	float					_font_10_digit_height	= 0.f;
	float					_font_13_digit_height	= 0.f;
	float					_font_16_digit_height	= 0.f;
	float					_font_18_digit_height	= 0.f;
	float					_font_20_digit_height	= 0.f;
	QColor					_autopilot_color		= { 250, 20, 255 };
	QColor					_navigation_color		= { 60, 255, 40 };
	QColor					_warning_color_1		= { 255, 40, 40 };
	QColor					_warning_color_2		= { 255, 200, 50 };
	QColor					_silver					= { 0xcc, 0xca, 0xc2 };
	QColor					_std_cyan				= { 0x00, 0xcc, 0xff };
	float					_height_for_width		= 1.f;
	float					_master_pen_scale		= 1.f;
	float					_master_font_scale		= 1.f;
	QPen					_autopilot_pen_1;
	QPen					_autopilot_pen_2;
	float					_w						= 0.f;
	float					_h						= 0.f;
	float					_window_w				= 0.f;
	float					_window_h				= 0.f;
	QRectF					_rect;

	static const char		DIGITS[10];
	static const char*		MINUS_SIGN;
};


inline
InstrumentAids::Token::Token (QPainter* painter, QPaintDevice* device):
	_painter (painter)
{
	_painter->begin (device);
}


inline
InstrumentAids::Token::~Token()
{
	_painter->end();
}


inline Painter&
InstrumentAids::painter() noexcept
{
	return _painter;
}


inline float
InstrumentAids::wh() const
{
	return std::min (_height_for_width * _w, _h);
}


inline float
InstrumentAids::win_wh() const
{
	return std::min (_window_w, _window_h);
}


inline QPen
InstrumentAids::get_pen (QColor const& color, float width, Qt::PenStyle style, Qt::PenCapStyle cap, Qt::PenJoinStyle join) const
{
	QPen pen (color, pen_width (width), style, cap, join);
	pen.setMiterLimit (0.25);
	return pen;
}


inline float
InstrumentAids::pen_width (float scale) const
{
	return std::max (0.f, 1.66f * _master_pen_scale * scale);
}


inline float
InstrumentAids::font_size (float scale) const
{
	return std::max (1.f, 1.26f * _master_font_scale * scale);
}


inline void
InstrumentAids::centrify (QRectF& rectf)
{
	rectf.translate (-0.5f * rectf.width(), -0.5f * rectf.height());
}


inline float
InstrumentAids::arc_degs (Angle deg)
{
	return (-16.0 * (deg - 90_deg)).quantity<Degree>();
}


inline float
InstrumentAids::arc_span (Angle deg)
{
	return (-16.0 * deg).quantity<Degree>();
}

} // namespace xf

#endif

