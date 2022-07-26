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
#include "gear.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


Gear::Gear (xf::Graphics const& graphics, std::string_view const& instance):
	GearIO (instance),
	InstrumentSupport (graphics)
{
	_inputs_observer.set_callback ([&] {
		mark_dirty();
	});
	_inputs_observer.observe ({
		&_io.requested_down,
		&_io.nose_up,
		&_io.nose_down,
		&_io.left_up,
		&_io.left_down,
		&_io.right_up,
		&_io.right_down,
	});
}


void
Gear::process (xf::Cycle const& cycle)
{
	_inputs_observer.process (cycle.update_time());
}


std::packaged_task<void()>
Gear::paint (xf::PaintRequest paint_request) const
{
	PaintingParams params;
	params.requested_down = _io.requested_down.get_optional();
	params.nose_up = _io.nose_up.get_optional();
	params.nose_down = _io.nose_down.get_optional();
	params.left_up = _io.left_up.get_optional();
	params.left_down = _io.left_down.get_optional();
	params.right_up = _io.right_up.get_optional();
	params.right_down = _io.right_down.get_optional();

	return std::packaged_task<void()> ([this, pr = std::move (paint_request), pp = std::move (params)] {
		async_paint (pr, pp);
	});
}


void
Gear::async_paint (xf::PaintRequest const& paint_request, PaintingParams const& pp) const
{
	auto aids = get_aids (paint_request);
	auto painter = get_painter (paint_request);

	bool v_requested_down = pp.requested_down.value_or (false);
	bool v_nose_up = pp.nose_up.value_or (false);
	bool v_nose_down = pp.nose_down.value_or (false);
	bool v_left_up = pp.left_up.value_or (false);
	bool v_left_down = pp.left_down.value_or (false);
	bool v_right_up = pp.right_up.value_or (false);
	bool v_right_down = pp.right_down.value_or (false);

	// If everything retracted according to setting, hide the widget:
	if (pp.requested_down && !v_requested_down &&
		v_nose_up && !v_nose_down &&
		v_left_up && !v_left_down &&
		v_right_up && !v_right_down)
	{
		return;
	}

	QColor const cyan { 0x44, 0xdd, 0xff };
	QColor const gray { 0xaa, 0xaa, 0xaa };
	QFont const box_font = aids->font_3;
	QFont const label_font = aids->font_2;
	QFontMetricsF const box_metrics (box_font);
	QFontMetricsF const label_metrics (label_font);

	// Positioning:
	painter.setFont (box_font);
	float const vmargin = -0.015f * box_metrics.height();
	float const hmargin = 0.1f * box_metrics.height();
	QRectF box = painter.get_text_box (QPointF (0.f, 0.f), Qt::AlignHCenter | Qt::AlignVCenter, "DOWN");
	box.adjust (-hmargin, -vmargin, hmargin, vmargin);

	auto paint_graybox = [&]() -> void {
		painter.setFont (box_font);
		painter.setPen (aids->get_pen (gray, 1.0f));

		float z = 0.61f * box_metrics.height();
		float d = 1.5f * z;

		// Painting:
		painter.setClipping (false);
		painter.drawRect (box);
		painter.setClipRect (box);
		for (float x = box.left(); x - d <= box.right(); x += z)
			painter.drawLine (QPointF (x, box.top()), QPointF (x - d, box.bottom()));
	};

	auto paint_down = [&]() -> void {
		painter.setFont (box_font);
		painter.setPen (aids->get_pen (Qt::green, 1.0f));

		// Painting:
		painter.setClipping (false);
		painter.fast_draw_text (box, Qt::AlignHCenter | Qt::AlignVCenter, "DOWN");
		painter.drawRect (box);
	};

	auto should_paint_graybox = [&](bool gear_up, bool gear_down) -> bool {
		return !pp.requested_down || (v_requested_down && (gear_up || !gear_down)) || (!v_requested_down && (gear_down || !gear_up));
	};

	painter.setBrush (Qt::NoBrush);
	painter.translate (0.5f * aids->width(), 0.5f * aids->height());
	QTransform center_transform = painter.transform();

	painter.translate (0.f, 1.5f * box.height());
	painter.setFont (label_font);
	painter.setPen (aids->get_pen (cyan, 1.f));
	painter.fast_draw_text (QPointF (0.f, 0.f), Qt::AlignHCenter | Qt::AlignTop, "GEAR");

	painter.setTransform (center_transform);
	painter.translate (0.f, -1.3f * box.bottom());

	if (should_paint_graybox (v_nose_up, v_nose_down))
		paint_graybox();
	else if (v_nose_down)
		paint_down();

	painter.setTransform (center_transform);
	painter.translate (-0.6f * box.width(), 0.75f * box.height());

	if (should_paint_graybox (v_left_up, v_left_down))
		paint_graybox();
	else if (v_left_down)
		paint_down();

	painter.setTransform (center_transform);
	painter.translate (+0.6f * box.width(), 0.75f * box.height());

	if (should_paint_graybox (v_right_up, v_right_down))
		paint_graybox();
	else if (v_right_down)
		paint_down();
}

