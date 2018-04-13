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

#ifndef XEFIS__SUPPORT__INSTRUMENT__INSTRUMENT_AIDS_H__INCLUDED
#define XEFIS__SUPPORT__INSTRUMENT__INSTRUMENT_AIDS_H__INCLUDED

// Standard:
#include <cstddef>
#include <optional>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/instrument/instrument_painter.h>
#include <xefis/support/instrument/text_painter.h>
#include <xefis/utility/strong_type.h>


namespace xf {

using WidthForHeight = StrongType<float, struct WidthForHeightType>;


class InstrumentAids
{
  public:
	class FontInfo
	{
	  public:
		QFont	font;
		float	digit_width;
		float	digit_height;

	  public:
		// Ctor
		explicit
		FontInfo (QFont const& font);

		operator QFont const&() const noexcept;

	  public:
		static float
		get_digit_width (QFont const& font);

		static float
		get_digit_height (QFont const& font);
	};

	static constexpr char			DIGITS[10]				{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	static constexpr char32_t		MINUS_SIGN				{ U'−' };
	static constexpr char const*	MINUS_SIGN_STR_UTF8		{ "−" };
	static constexpr QColor			kAutopilotColor			{ 250, 20, 255 };
	static constexpr QColor			kNavigationColor		{ 60, 255, 40 };
	static constexpr QColor			kCautionColor			{ 255, 200, 50 };
	static constexpr QColor			kWarningColor			{ 255, 40, 40 };
	static constexpr QColor			kSilver					{ 0xcc, 0xca, 0xc2 };
	static constexpr QColor			kCyan					{ 0x00, 0xcc, 0xff };

  public:
	// Ctor
	explicit
	InstrumentAids();

	/**
	 * Return an instrument painter with pointers to local caches, etc.
	 * Not thread safe!
	 */
	InstrumentPainter
	get_painter (QPaintDevice&) const;

	/**
	 * Return font modified to have given xf::FontSize.
	 */
	static QFont
	resized (QFont const&, xf::FontSize);

	/**
	 * Return a centered rect inside whole_area that hold given
	 * width-for-height ratio.
	 */
	static QRectF
	centered_rect (QRectF whole_area, WidthForHeight);

	/**
	 * Translate the rect so that its old top-left corner becomes its center.
	 */
	static void
	centrify (QRectF& rectf);

	/**
	 * Convert angle to units proper for QPainter::drawArc (...) and QPainter::drawChord (...) functions.
	 */
	static float
	angle_for_qpainter (Angle deg);

	/**
	 * Return value to use as pen width.
	 */
	float
	pen_width (float scale) const;

	/**
	 * Return value to use as font pixel size.
	 */
	float
	font_pixel_size (float scale) const;

	/**
	 * Return pen suitable for instrument drawing.
	 */
	QPen
	get_pen (QColor const& color, float width, Qt::PenStyle style = Qt::SolidLine, Qt::PenCapStyle cap = Qt::RoundCap, Qt::PenJoinStyle join = Qt::MiterJoin) const;

  private:
	QFont							_default_font;
	TextPainter::Cache mutable		_text_painter_cache; // FIXME this implies thread-safety:
	std::optional<WidthForHeight>	_aspect_ratio;

  protected:
	FontInfo	font_0;
	FontInfo	font_1;
	FontInfo	font_2;
	FontInfo	font_3;
	FontInfo	font_4;
	FontInfo	font_5;
	QPen		autopilot_pen_1;
	QPen		autopilot_pen_2;
};


inline
InstrumentAids::FontInfo::operator QFont const&() const noexcept
{
	return font;
}


inline void
InstrumentAids::centrify (QRectF& rectf)
{
	rectf.translate (-0.5f * rectf.width(), -0.5f * rectf.height());
}


inline float
InstrumentAids::angle_for_qpainter (Angle deg)
{
	return 16 * deg.in<Degree>();
}


inline float
InstrumentAids::pen_width (float scale) const
{
	return std::max (0.0f, 1.66f * scale * _master_pen_scale);
}


inline float
InstrumentAids::font_pixel_size (float scale) const
{
	return std::max (1.0f, 1.26f * _master_font_scale * scale);
}


inline QPen
InstrumentAids::get_pen (QColor const& color, float width, Qt::PenStyle style, Qt::PenCapStyle cap, Qt::PenJoinStyle join) const
{
	QPen pen { color, pen_width (width), style, cap, join };
	pen.setMiterLimit (0.25);
	return pen;
}

} // namespace xf

#endif

