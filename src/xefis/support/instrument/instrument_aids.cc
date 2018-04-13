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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/services.h>

// Local:
#include "instrument_aids.h"


namespace xf {

InstrumentAids::FontInfo::FontInfo (QFont const& font):
	font (font),
	digit_width (get_digit_width (font)),
	digit_height (get_digit_height (font))
{ }


float
InstrumentAids::FontInfo::get_digit_width (QFont const& font)
{
	QFontMetricsF font_metrics (font);
	float digit_width = 0;

	for (char c: InstrumentAids::DIGITS)
		digit_width = std::max<float> (digit_width, font_metrics.width (c));

	return digit_width;
}


float
InstrumentAids::FontInfo::get_digit_height (QFont const& font)
{
	constexpr float scale_down_line_height_factor = 0.7;
	return scale_down_line_height_factor * QFontMetricsF (font).height();
}


InstrumentAids::InstrumentAids():
	_default_font (xf::Services::instrument_font()),
	font_0 (resized (_default_font, 10.0f)),
	font_1 (resized (_default_font, 11.0f)),
	font_2 (resized (_default_font, 13.0f)),
	font_3 (resized (_default_font, 16.0f)),
	font_4 (resized (_default_font, 18.0f)),
	font_5 (resized (_default_font, 20.0f)),
	autopilot_pen_1 (get_pen (kAutopilotColor.darker (300), 1.8f)),
	autopilot_pen_2 (get_pen (kAutopilotColor, 1.25f))
{ }


InstrumentPainter
InstrumentAids::get_painter (QPaintDevice& canvas) const
{
	return xf::InstrumentPainter (canvas, _text_painter_cache);
}


QFont
InstrumentPainter::resized (QFont const& font, xf::FontSize font_size)
{
	QFont copy { font };
	copy.setPixelSize (*font_size);
	return copy;
}


QRectF
InstrumentAids::centered_rect (QRectF input_rect, WidthForHeight width_for_height)
{
	float const input_width_for_height = input_rect.width() / input_rect.height();
	float remove_horizontal = 0.0f;
	float remove_vertical = 0.0f;

	if (*width_for_height > input_width_for_height)
	{
		float const new_height = input_rect.width() / *width_for_height;
		remove_vertical = 0.5f * (input_rect.height() - new_height);
	}
	else
	{
		float const new_width = input_rect.height() * *width_for_height;
		remove_horizontal = 0.5f * (input_rect.width() - new_width);
	}

	return input_rect.marginsRemoved ({ remove_horizontal, remove_vertical, remove_horizontal, remove_vertical });
}

} // namespace xf

