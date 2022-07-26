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

// Local:
#include "instrument_aids.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


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

	for (char c: InstrumentAids::kDigits)
		digit_width = std::max<float> (digit_width, font_metrics.width (c));

	return digit_width;
}


float
InstrumentAids::FontInfo::get_digit_height (QFont const& font)
{
	constexpr float scale_down_line_height_factor = 0.7;
	return scale_down_line_height_factor * QFontMetricsF (font).height();
}


InstrumentAids::InstrumentAids (PaintRequest::Metric const& canvas_metric, Graphics const& graphics):
	_graphics (graphics),
	_canvas_metric (canvas_metric),
	font_0 (resized (graphics.instrument_font(), 1.0f * _canvas_metric.font_height())),
	font_1 (resized (graphics.instrument_font(), 1.1f * _canvas_metric.font_height())),
	font_2 (resized (graphics.instrument_font(), 1.3f * _canvas_metric.font_height())),
	font_3 (resized (graphics.instrument_font(), 1.6f * _canvas_metric.font_height())),
	font_4 (resized (graphics.instrument_font(), 1.8f * _canvas_metric.font_height())),
	font_5 (resized (graphics.instrument_font(), 2.0f * _canvas_metric.font_height())),
	autopilot_pen_1 (get_pen (kAutopilotDarkColor, 1.8f)),
	autopilot_pen_2 (get_pen (kAutopilotColor, 1.25f))
{ }


QFont
InstrumentAids::resized (QFont const& font, si::Length height) const
{
	return resized (font, height, _canvas_metric.pixel_density());
}


QFont
InstrumentAids::resized (QFont const& font, si::Length height, si::PixelDensity pixel_density)
{
	QFont copy { font };
	copy.setPixelSize (pixels (height, pixel_density));
	return copy;
}


QFont
InstrumentAids::resized (QFont const& font, FontPixelSize font_pixel_size)
{
	QFont copy { font };
	copy.setPixelSize (*font_pixel_size);
	return copy;
}


QFont
InstrumentAids::scaled_default_font (float scale) const
{
	return resized (_graphics.instrument_font(), scale * _canvas_metric.font_height());
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

