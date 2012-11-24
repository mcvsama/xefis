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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/services.h>

// Local:
#include "instrument_widget.h"


namespace Xefis {

const char	InstrumentWidget::DIGITS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
const char*	InstrumentWidget::MINUS_SIGN = "−";


InstrumentWidget::InstrumentWidget (QWidget* parent):
	QWidget (parent)
{
	_font = Xefis::Services::instrument_font();
	_autopilot_color = QColor (250, 120, 255);
	_navigation_color = QColor (40, 255, 40);

	update_fonts();
}


void
InstrumentWidget::resizeEvent (QResizeEvent*)
{
	update_fonts();
}


int
InstrumentWidget::get_digit_width (QFont& font) const
{
	QFontMetrics font_metrics (font);
	int digit_width = 0;
	for (char c: DIGITS)
		digit_width = std::max (digit_width, font_metrics.width (c));
	return digit_width;
}


void
InstrumentWidget::update_fonts()
{
	float const height_scale_factor = 0.7f;

	_font_10_bold = _font;
	_font_10_bold.setPixelSize (font_size (10.f));
	_font_10_bold.setBold (true);
	_font_10_digit_width = get_digit_width (_font_10_bold);
	_font_10_digit_height = height_scale_factor * QFontMetrics (_font_10_bold).height();

	_font_13_bold = _font;
	_font_13_bold.setPixelSize (font_size (13.f));
	_font_13_bold.setBold (true);
	_font_13_digit_width = get_digit_width (_font_13_bold);
	_font_13_digit_height = QFontMetrics (_font_13_bold).height();
	_font_13_digit_height = height_scale_factor * QFontMetrics (_font_13_bold).height();

	_font_16_bold = _font;
	_font_16_bold.setPixelSize (font_size (16.f));
	_font_16_bold.setBold (true);
	_font_16_digit_width = get_digit_width (_font_16_bold);
	_font_16_digit_height = QFontMetrics (_font_16_bold).height();
	_font_16_digit_height = height_scale_factor * QFontMetrics (_font_16_bold).height();

	_font_20_bold = _font;
	_font_20_bold.setPixelSize (font_size (20.f));
	_font_20_bold.setBold (true);
	_font_20_digit_width = get_digit_width (_font_20_bold);
	_font_20_digit_height = QFontMetrics (_font_20_bold).height();
	_font_20_digit_height = height_scale_factor * QFontMetrics (_font_20_bold).height();
}

} // namespace Xefis

