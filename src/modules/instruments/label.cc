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
#include <xefis/core/window.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/string.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/qdom_iterator.h>

// Local:
#include "label.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/label", Label)


Label::Label (xf::ModuleManager* module_manager, QDomElement const& config):
	xf::Instrument (module_manager, config),
	InstrumentAids (1.f)
{
	for (QDomElement const& e: xf::iterate_sub_elements (config))
	{
		if (e == "label")
			_label = e.text();
		else if (e == "align")
			_alignment = xf::parse_alignment (e.text());
		else if (e == "color")
			_color = xf::parse_color (e.text());
		else if (e == "font-size")
			_font_size = e.text().toDouble();
	}
}


void
Label::resizeEvent (QResizeEvent*)
{
	auto xw = dynamic_cast<xf::Window*> (window());
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
	font.setPixelSize (font_size (_font_size));
	painter().setFont (font);
	painter().setPen (_color);
	painter().fast_draw_text (rect(), _alignment, _label);
}

