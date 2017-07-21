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
#include <xefis/core/v1/window.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/string.h>

// Local:
#include "label.h"


Label::Label (std::string const& instance):
	Instrument (instance),
	InstrumentAids (1.f)
{ }


void
Label::resizeEvent (QResizeEvent*)
{
	auto xw = dynamic_cast<v1::Window*> (window());
	if (xw)
		set_scaling (xw->pen_scale(), xw->font_scale());

	InstrumentAids::update_sizes (size(), window()->size());
}


void
Label::paintEvent (QPaintEvent*)
{
	auto paint_token = get_token (this);
	clear_background();

	QFont font (_font_10);
	font.setPixelSize (**this->font_size);
	painter().setFont (font);
	painter().setPen (*this->color);
	painter().fast_draw_text (rect(), *this->alignment, *this->label);
}

