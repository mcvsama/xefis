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
#include <xefis/utility/numeric.h>

// Local:
#include "gear.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/gear", Gear)


Gear::Gear (v1::ModuleManager* module_manager, QDomElement const& config):
	Instrument (module_manager, config),
	InstrumentAids (0.5f)
{
	parse_properties (config, {
		{ "setting.down", _setting_down, true },
		{ "nose.up", _nose_up, true },
		{ "nose.down", _nose_down, true },
		{ "left.up", _left_up, true },
		{ "left.down", _left_down, true },
		{ "right.up", _right_up, true },
		{ "right.down", _right_down, true },
	});

	update();
}


void
Gear::data_updated()
{
	if (_setting_down.fresh() ||
		_nose_up.fresh() ||
		_nose_down.fresh() ||
		_left_up.fresh() ||
		_left_down.fresh() ||
		_right_up.fresh() ||
		_right_down.fresh())
	{
		update();
	}
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

	bool setting_down = _setting_down.read (false);
	bool setting_invalid = _setting_down.is_nil();
	bool nose_up = _nose_up.read (false);
	bool nose_down = _nose_down.read (false);
	bool left_up = _left_up.read (false);
	bool left_down = _left_down.read (false);
	bool right_up = _right_up.read (false);
	bool right_down = _right_down.read (false);

	// If everything retracted according to setting, hide the widget:
	if (_setting_down.valid() && !setting_down &&
		nose_up && !nose_down &&
		left_up && !left_down &&
		right_up && !right_down)
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
		return setting_invalid || (setting_down && (gear_up || !gear_down)) || (!setting_down && (gear_down || !gear_up));
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

	if (should_paint_graybox (nose_up, nose_down))
		paint_graybox();
	else if (nose_down)
		paint_down();

	painter().setTransform (center_transform);
	painter().translate (-0.6f * box.width(), 0.75f * box.height());
	if (should_paint_graybox (left_up, left_down))
		paint_graybox();
	else if (left_down)
		paint_down();

	painter().setTransform (center_transform);
	painter().translate (+0.6f * box.width(), 0.75f * box.height());
	if (should_paint_graybox (right_up, right_down))
		paint_graybox();
	else if (right_down)
		paint_down();
}

