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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/screen.h>
#include <xefis/support/instrument/shadow.h>
#include <xefis/utility/types.h>

// Neutrino:
#include <neutrino/strong_type.h>

// Qt:
#include <QPen>
#include <QFont>

// Standard:
#include <cstddef>
#include <optional>


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

	static constexpr char			kDigits[10]				{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	static constexpr char32_t		kMinusSign				{ U'−' };
	static constexpr char const*	kMinusSignStrUTF8		{ "−" };
	static inline QColor const		kAutopilotColor			{ 252, 40, 255 };
	static inline QColor const		kAutopilotDarkColor		{ kAutopilotColor.darker (300) };
	static inline QColor const		kNavigationColor		{ 60, 255, 40 };
	static inline QColor const		kCautionColor			{ 255, 200, 50 };
	static inline QColor const		kWarningColor			{ 255, 40, 40 };
	static inline QColor const		kSilver					{ 0xcc, 0xca, 0xc2 };
	static inline QColor const		kCyan					{ 0x00, 0xcc, 0xff };

  public:
	// Ctor
	explicit
	InstrumentAids (PaintRequest::Metric const&, Graphics const&);

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

	/**
	 * Shortcut width accessor.
	 */
	int
	width() const;

	/**
	 * Shortcut height accessor.
	 */
	int
	height() const;

	/**
	 * Return smaller of the width/height sizes.
	 */
	int
	lesser_dimension() const;

	/**
	 * Return greater of the width/height sizes.
	 */
	int
	greater_dimension() const;

	/**
	 * Return number of pixels for given Length and current pixel density.
	 */
	float
	pixels (si::Length);

	/**
	 * Return number of pixels for given Length given pixel density.
	 */
	static float
	pixels (si::Length, si::PixelDensity);

	/**
	 * Return font modified to have given height.
	 */
	QFont
	resized (QFont const& font, si::Length height) const;

	/**
	 * Return font modified to have given height.
	 */
	static QFont
	resized (QFont const& font, si::Length height, si::PixelDensity);

	/**
	 * Return font modified to have given xf::FontPixelSize.
	 */
	static QFont
	resized (QFont const&, FontPixelSize);

	/**
	 * Scale and return default instrument font.
	 */
	QFont
	scaled_default_font (float scale) const;

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
	static constexpr float
	angle_for_qpainter (si::Angle deg);

	/**
	 * Return default shadow.
	 */
	Shadow
	default_shadow() const;

  private:
	Graphics const&					_graphics;
	std::optional<WidthForHeight>	_aspect_ratio;
	PaintRequest::Metric			_canvas_metric;

  public:
	FontInfo const	font_0;
	FontInfo const	font_1;
	FontInfo const	font_2;
	FontInfo const	font_3;
	FontInfo const	font_4;
	FontInfo const	font_5;
	QPen const		autopilot_pen_1;
	QPen const		autopilot_pen_2;
};


inline
InstrumentAids::FontInfo::operator QFont const&() const noexcept
{
	return font;
}


inline float
InstrumentAids::pen_width (float scale) const
{
	return std::max (0.0f, pixels (scale * _canvas_metric.pen_width(), _canvas_metric.pixel_density()));
}


inline float
InstrumentAids::font_pixel_size (float scale) const
{
	return std::max (1.0f, pixels (scale * _canvas_metric.font_height(), _canvas_metric.pixel_density()));
}


inline QPen
InstrumentAids::get_pen (QColor const& color, float width, Qt::PenStyle style, Qt::PenCapStyle cap, Qt::PenJoinStyle join) const
{
	QPen pen { color, pen_width (width), style, cap, join };
	pen.setMiterLimit (0.25);
	return pen;
}


inline int
InstrumentAids::width() const
{
	return _canvas_metric.canvas_size().width();
}


inline int
InstrumentAids::height() const
{
	return _canvas_metric.canvas_size().height();
}


inline int
InstrumentAids::lesser_dimension() const
{
	return std::min (width(), height());
}


inline int
InstrumentAids::greater_dimension() const
{
	return std::max (width(), height());
}


inline float
InstrumentAids::pixels (si::Length length)
{
	return pixels (length, _canvas_metric.pixel_density());
}


inline float
InstrumentAids::pixels (si::Length length, si::PixelDensity pixel_density)
{
	return length * pixel_density;
}


inline void
InstrumentAids::centrify (QRectF& rectf)
{
	rectf.translate (-0.5f * rectf.width(), -0.5f * rectf.height());
}


constexpr float
InstrumentAids::angle_for_qpainter (si::Angle deg)
{
	return 16 * deg.in<si::Degree>();
}


inline Shadow
InstrumentAids::default_shadow() const
{
	Shadow shadow;
	shadow.set_width (pen_width (0.75f));
	return shadow;
}


/*
 * Global functions
 */


namespace literals {

constexpr float
operator"" _qdeg (long double angle)
{
	return InstrumentAids::angle_for_qpainter (1_deg * angle);
}


constexpr float
operator"" _qdeg (unsigned long long angle)
{
	return InstrumentAids::angle_for_qpainter (1_deg * angle);
}

} // namespace literals
} // namespace xf

#endif

