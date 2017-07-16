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
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/window.h>

// Local:
#include "group_box.h"


namespace xf {

GroupBox::GroupBox (QString const& label, QWidget* parent):
	QWidget (parent),
	InstrumentAids (1.0),
	_label (label)
{
	_frame_color = _frame_color.darker (100);
}


void
GroupBox::set_padding (std::array<int, 4> const& padding)
{
	_padding = padding;
}


void
GroupBox::resizeEvent (QResizeEvent*)
{
	auto xw = dynamic_cast<xf::Window*> (window());
	if (xw)
		set_scaling (xw->pen_scale(), xw->font_scale());

	InstrumentAids::update_sizes (size(), window()->size());
}


void
GroupBox::paintEvent (QPaintEvent*)
{
	auto painting_token = get_token (this);
	clear_background();

	// Margins:
	QFont font = _font_10;
	font.setPixelSize (font_size (15.0));
	painter().setFont (font);
	QFontMetricsF metrics (painter().font());
	int av = metrics.height();
	int ah = av;
	setContentsMargins (ah + _padding[0], av + _padding[1], ah + _padding[2], av + _padding[3]);

	// Rounded rectangle:
	float bh = 0.5 * ah;
	float bv = 0.5 * av;
	float r = pen_width (5.0);
	painter().setBrush (Qt::NoBrush);
	painter().setPen (get_pen (_frame_color, 1.0));
	painter().drawRoundedRect (rect().adjusted (bh, bv, -bh, -bv), r, r);

	// Label
	QPointF text_hook (0.5 * width(), 0.5 * av);
	Qt::Alignment alignment = Qt::AlignVCenter | Qt::AlignHCenter;
	QRectF label_rect = painter().get_text_box (text_hook, alignment, _label);
	label_rect.adjust (-0.5 * av, 0, 0.5 * av, 0);
	painter().setPen (Qt::NoPen);
	painter().setBrush (Qt::black);
	painter().drawRect (label_rect);
	painter().setPen (get_pen (_label_color, 1.0));
	painter().fast_draw_text (text_hook, alignment, _label);
}

} // namespace xf

