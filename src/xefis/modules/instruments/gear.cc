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
#include <xefis/core/v1/window.h>
#include <xefis/utility/numeric.h>

// Local:
#include "gear.h"


Gear::Gear (std::unique_ptr<GearIO> module_io, std::string const& instance):
	Instrument (std::move (module_io), instance),
	InstrumentAids (0.5f)
{
	_inputs_observer.set_callback ([&]{ update(); });
	_inputs_observer.observe ({
		&io.requested_down,
		&io.nose_up,
		&io.nose_down,
		&io.left_up,
		&io.left_down,
		&io.right_up,
		&io.right_down,
	});
}


void
Gear::process (xf::Cycle const& cycle)
{
	_inputs_observer.process (cycle.update_dt());
}


void
Gear::resizeEvent (QResizeEvent*)
{
	auto xw = dynamic_cast<v1::Window*> (window());
	if (xw)
		set_scaling (xw->pen_scale(), xw->font_scale());

	InstrumentAids::update_sizes (size(), window()->size());
}


void
Gear::paintEvent (QPaintEvent*)
{
	auto painting_token = get_token (this);
	clear_background();

	bool v_requested_down = io.requested_down.value_or (false);
	bool v_nose_up = io.nose_up.value_or (false);
	bool v_nose_down = io.nose_down.value_or (false);
	bool v_left_up = io.left_up.value_or (false);
	bool v_left_down = io.left_down.value_or (false);
	bool v_right_up = io.right_up.value_or (false);
	bool v_right_down = io.right_down.value_or (false);

	// If everything retracted according to setting, hide the widget:
	if (io.requested_down && !v_requested_down &&
		v_nose_up && !v_nose_down &&
		v_left_up && !v_left_down &&
		v_right_up && !v_right_down)
	{
		return;
	}

	QColor cyan { 0x44, 0xdd, 0xff };
	QColor gray { 0xaa, 0xaa, 0xaa };
	QFont box_font = _font_16;
	QFont label_font = _font_13;
	QFontMetricsF box_metrics (box_font);
	QFontMetricsF label_metrics (label_font);

	// Positioning:
	painter().setFont (box_font);
	float vmargin = -0.015f * box_metrics.height();
	float hmargin = 0.1f * box_metrics.height();
	QRectF box = painter().get_text_box (QPointF (0.f, 0.f), Qt::AlignHCenter | Qt::AlignVCenter, "DOWN");
	box.adjust (-hmargin, -vmargin, hmargin, vmargin);

	auto paint_graybox = [&]() -> void {
		painter().setFont (box_font);
		painter().setPen (get_pen (gray, 1.2f));

		float z = 0.61f * box_metrics.height();
		float d = 1.5f * z;

		// Painting:
		painter().setClipping (false);
		painter().drawRect (box);
		painter().setClipRect (box);
		for (float x = box.left(); x - d <= box.right(); x += z)
			painter().drawLine (QPointF (x, box.top()), QPointF (x - d, box.bottom()));
	};

	auto paint_down = [&]() -> void {
		painter().setFont (box_font);
		painter().setPen (get_pen (Qt::green, 1.2f));

		// Painting:
		painter().setClipping (false);
		painter().fast_draw_text (box, Qt::AlignHCenter | Qt::AlignVCenter, "DOWN");
		painter().drawRect (box);
	};

	auto should_paint_graybox = [&](bool gear_up, bool gear_down) -> bool {
		return io.requested_down.is_nil() || (v_requested_down && (gear_up || !gear_down)) || (!v_requested_down && (gear_down || !gear_up));
	};

	painter().setBrush (Qt::NoBrush);
	painter().translate (0.5f * width(), 0.5f * height());
	QTransform center_transform = painter().transform();

	painter().translate (0.f, 1.5f * box.height());
	painter().setFont (label_font);
	painter().setPen (get_pen (cyan, 1.f));
	painter().fast_draw_text (QPointF (0.f, 0.f), Qt::AlignHCenter | Qt::AlignTop, "GEAR");

	painter().setTransform (center_transform);
	painter().translate (0.f, -1.3f * box.bottom());

	if (should_paint_graybox (v_nose_up, v_nose_down))
		paint_graybox();
	else if (v_nose_down)
		paint_down();

	painter().setTransform (center_transform);
	painter().translate (-0.6f * box.width(), 0.75f * box.height());
	if (should_paint_graybox (v_left_up, v_left_down))
		paint_graybox();
	else if (v_left_down)
		paint_down();

	painter().setTransform (center_transform);
	painter().translate (+0.6f * box.width(), 0.75f * box.height());
	if (should_paint_graybox (v_right_up, v_right_down))
		paint_graybox();
	else if (v_right_down)
		paint_down();
}

