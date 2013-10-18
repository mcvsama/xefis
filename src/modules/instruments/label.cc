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

// Qt:
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/string.h>
#include <xefis/utility/qdom.h>

// Local:
#include "label.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/label", Label);


Label::Label (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Xefis::Instrument (module_manager, config),
	InstrumentAids (1.f)
{
	for (QDomElement& e: config)
	{
		if (e == "label")
			_label = e.text();
		else if (e == "align")
		{
			QStringList list = e.text().split (" ");
			_alignment = 0;

			if (list.contains ("top"))
				_alignment |= Qt::AlignTop;
			else if (list.contains ("vcenter"))
				_alignment |= Qt::AlignVCenter;
			else if (list.contains ("bottom"))
				_alignment |= Qt::AlignBottom;

			if (list.contains ("left"))
				_alignment |= Qt::AlignLeft;
			else if (list.contains ("hcenter"))
				_alignment |= Qt::AlignHCenter;
			else if (list.contains ("right"))
				_alignment |= Qt::AlignRight;
		}
		else if (e == "color")
			_color = Xefis::parse_color (e.text());
		else if (e == "font-size")
			_font_size = e.text().toFloat();
	}
}


void
Label::paintEvent (QPaintEvent*)
{
	Xefis::Painter painter (this, &_text_painter_cache);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	painter.setRenderHint (QPainter::NonCosmeticDefaultPen, true);

	QFont font (_font_10);
	font.setPixelSize (font_size (_font_size));
	painter.setFont (font);
	painter.setBrush (Qt::black);
	painter.setPen (Qt::NoPen);
	painter.drawRect (rect());
	painter.setPen (_color);
	painter.fast_draw_text (rect(), _alignment, _label);
}

