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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>

// Local:
#include "flaps.h"


Flaps::Flaps (std::unique_ptr<FlapsIO> module_io, xf::Graphics const& graphics, std::string_view const& instance):
	Instrument (std::move (module_io), instance),
	InstrumentSupport (graphics)
{
	_inputs_observer.set_callback ([&]{
		mark_dirty();
	});
	_inputs_observer.observe ({
		&io.current_angle,
		&io.set_angle,
	});
}


void
Flaps::process (xf::Cycle const& cycle)
{
	_inputs_observer.process (cycle.update_time());
}


std::packaged_task<void()>
Flaps::paint (xf::PaintRequest paint_request) const
{
	return std::packaged_task<void()> ([&, pr = std::move (paint_request)] {
		async_paint (pr);
	});
}


void
Flaps::async_paint (xf::PaintRequest const& paint_request) const
{
	auto aids = get_aids (paint_request);
	auto painter = get_painter (paint_request);

	if (*io.hide_retracted)
		if (io.current_angle && *io.current_angle < 0.1_deg)
			if (io.set_angle && *io.set_angle < 0.5_deg)
				return;

	QColor cyan { 0x44, 0xdd, 0xff };
	QFont setting_font = aids->font_3.font;
	QFont label_font = aids->font_2.font;

	float block_height = aids->height() - QFontMetricsF (setting_font).height();
	float block_width = 6.0 / 40.0 * block_height;
	QRectF block (0.f, 0.f, block_width, block_height);
	aids->centrify (block);

	painter.translate (0.5f * aids->width(), 0.5f * aids->height());

	// Cyan vertical text:
	painter.setFont (label_font);
	painter.setPen (cyan);
	painter.fast_draw_vertical_text (QPointF (block.left() - QFontMetricsF (label_font).width ("0"), 0.f), Qt::AlignVCenter | Qt::AlignRight, "FLAPS");

	// Flaps white box:
	painter.setPen (aids->get_pen (Qt::white, 1.f));
	painter.setBrush (Qt::NoBrush);
	painter.drawRect (block);

	// Filled block showing current value:
	if (io.current_angle)
	{
		Angle current = xf::clamped<Angle> (*io.current_angle, 0_deg, *io.maximum_angle);
		QRectF filled_block = block;
		filled_block.setHeight (current / *io.maximum_angle * filled_block.height());
		painter.setPen (Qt::NoPen);
		painter.setBrush (Qt::white);
		painter.drawRect (filled_block);
	}

	// Target setting in green:
	if (io.set_angle)
	{
		// Green line:
		Angle setting = xf::clamped<Angle> (*io.set_angle, 0_deg, *io.maximum_angle);
		float w = 0.3f * block.width();
		float s = block.top() + setting / *io.maximum_angle * block.height();
		painter.setPen (aids->get_pen (Qt::green, 2.f));
		painter.paint (aids->default_shadow(), [&] {
			painter.drawLine (QPointF (block.left() - w, s), QPointF (block.right() + w, s));
		});

		// Number or UP
		QString number = "UP";
		if (setting > 0.5_deg)
			number = QString ("%1").arg (xf::symmetric_round (setting.in<Degree>()));
		painter.setFont (setting_font);
		painter.fast_draw_text (QPointF (block.right() + 2.f * w, s), Qt::AlignVCenter | Qt::AlignLeft, number);
	}
}

