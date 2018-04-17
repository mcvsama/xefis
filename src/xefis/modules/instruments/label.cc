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

// Qt:
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "label.h"


Label::Label (std::unique_ptr<LabelIO> module_io, std::string const& instance):
	Instrument (std::move (module_io), instance)
{ }


void
Label::paint (xf::PaintRequest& paint_request) const
{
	auto aids = get_aids (paint_request);
	auto painter = get_painter (paint_request);

	QFont font (aids->font_1);
	font.setPixelSize (aids->font_pixel_size (*io.font_scale));

	painter.setFont (font);
	painter.setPen (*io.color);
	painter.fast_draw_text (paint_request.canvas().rect(), *io.alignment, *io.label);
}

