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
#include <xefis/utility/string.h>

// Local:
#include "label.h"


Label::Label (std::unique_ptr<LabelIO> module_io, std::string const& instance):
	Instrument (std::move (module_io), instance)
{ }


void
Label::paint (QImage& canvas) const
{
	// TODO add support for static instruments like this (only changes when some kind of update() method is called,
	// eg. when canvas size changes.
	auto p = get_painter (canvas);
	QFont font (font_1);
	// TODO how to interpret this xf::FontSize?
	font.setPixelSize (**io.font_size);
	p.setFont (font);
	p.setPen (*io.color);
	p.fast_draw_text (canvas.rect(), *io.alignment, *io.label);
}

