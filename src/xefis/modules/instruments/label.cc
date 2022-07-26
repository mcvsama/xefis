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
#include "label.h"

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QtWidgets/QLayout>

// Standard:
#include <cstddef>


Label::Label (xf::Graphics const& graphics, std::string_view const& instance):
	LabelIO (instance),
	InstrumentSupport (graphics)
{ }


std::packaged_task<void()>
Label::paint (xf::PaintRequest paint_request) const
{
	PaintingParams params;
	params.font_scale = *_io.font_scale;
	params.label = *_io.label;
	params.color = *_io.color;
	params.alignment = *_io.alignment;

	return std::packaged_task<void()> ([this, pr = std::move (paint_request), pp = std::move (params)] {
		async_paint (pr, pp);
	});
}


void
Label::async_paint (xf::PaintRequest const& paint_request, PaintingParams const& pp) const
{
	auto aids = get_aids (paint_request);
	auto painter = get_painter (paint_request);

	QFont font (aids->font_1);
	font.setPixelSize (aids->font_pixel_size (pp.font_scale));

	painter.setFont (font);
	painter.setPen (pp.color);
	painter.fast_draw_text (paint_request.metric().canvas_rect(), pp.alignment, pp.label);
}

