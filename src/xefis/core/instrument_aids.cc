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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/services.h>

// Local:
#include "instrument_aids.h"


namespace Xefis {

const char	InstrumentAids::DIGITS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
const char*	InstrumentAids::MINUS_SIGN = "−";


InstrumentAids::InstrumentAids (float height_for_width, float master_pen_scale, float master_font_scale):
	_height_for_width (height_for_width),
	_master_pen_scale (master_pen_scale),
	_master_font_scale (master_font_scale)
{
	_font = Xefis::Services::instrument_font();
	_autopilot_color = QColor (250, 70, 255);
	_navigation_color = QColor (60, 255, 40);
}


void
InstrumentAids::update_sizes (QSize const& size, QSize const& window_size)
{
	_w = size.width();
	_h = size.height();
	_window_w = window_size.width();
	_window_h = window_size.height();

	float const font_height_scale_factor = 0.7f;

	_font_10 = _font;
	_font_10.setPixelSize (font_size (11.f));
	_font_10_digit_width = get_digit_width (_font_10);
	_font_10_digit_height = font_height_scale_factor * QFontMetrics (_font_10).height();

	_font_13 = _font;
	_font_13.setPixelSize (font_size (13.f));
	_font_13_digit_width = get_digit_width (_font_13);
	_font_13_digit_height = QFontMetrics (_font_13).height();
	_font_13_digit_height = font_height_scale_factor * QFontMetrics (_font_13).height();

	_font_16 = _font;
	_font_16.setPixelSize (font_size (16.f));
	_font_16_digit_width = get_digit_width (_font_16);
	_font_16_digit_height = QFontMetrics (_font_16).height();
	_font_16_digit_height = font_height_scale_factor * QFontMetrics (_font_16).height();

	_font_20 = _font;
	_font_20.setPixelSize (font_size (20.f));
	_font_20_digit_width = get_digit_width (_font_20);
	_font_20_digit_height = QFontMetrics (_font_20).height();
	_font_20_digit_height = font_height_scale_factor * QFontMetrics (_font_20).height();

	_autopilot_pen_1 = get_pen (_autopilot_color.darker (300), 1.65f);
	_autopilot_pen_2 = get_pen (_autopilot_color, 1.0f);
}


int
InstrumentAids::get_digit_width (QFont& font) const
{
	QFontMetrics font_metrics (font);
	int digit_width = 0;
	for (char c: DIGITS)
		digit_width = std::max (digit_width, font_metrics.width (c));
	return digit_width;
}


float
InstrumentAids::translate_descent (QFontMetricsF const& metrics_1, QFontMetricsF const& metrics_2)
{
	return metrics_2.height() - metrics_2.descent() - metrics_1.height() + metrics_1.descent();
}

} // namespace Xefis

