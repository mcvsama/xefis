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
#include "flaps.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/flaps", Flaps)


Flaps::Flaps (v1::ModuleManager* module_manager, QDomElement const& config):
	Instrument (module_manager, config),
	InstrumentAids (2.f)
{
	parse_settings (config, {
		{ "maximum", _maximum, true },
		{ "hide-retracted", _hide_retracted, false },
	});

	parse_properties (config, {
		{ "current", _current, true },
		{ "setting", _setting, true },
	});

	update();
}


void
Flaps::data_updated()
{
	if (_current.fresh() || _setting.fresh())
		update();
}


void
Flaps::resizeEvent (QResizeEvent*)
{
	auto xw = dynamic_cast<v1::Window*> (window());
	if (xw)
		set_scaling (xw->pen_scale(), xw->font_scale());

	InstrumentAids::update_sizes (size(), window()->size());
}


void
Flaps::paintEvent (QPaintEvent*)
{
	auto painting_token = get_token (this);
	clear_background();

	if (_hide_retracted)
		if (_current.valid() && *_current < 0.1_deg)
			if (_setting.valid() && *_setting < 0.5_deg)
				return;

	QColor cyan { 0x44, 0xdd, 0xff };
	QFont setting_font = _font_16;
	QFont label_font = _font_13;

	float block_height = height() - QFontMetrics (setting_font).height();
	float block_width = 6.0 / 40.0 * block_height;
	QRectF block (0.f, 0.f, block_width, block_height);
	centrify (block);

	painter().translate (0.5f * width(), 0.5f * height());

	// Cyan vertical text:
	painter().setFont (label_font);
	painter().setPen (cyan);
	painter().fast_draw_vertical_text (QPointF (block.left() - QFontMetrics (label_font).width ("0"), 0.f), Qt::AlignVCenter | Qt::AlignRight, "FLAPS");

	// Flaps white box:
	painter().setPen (get_pen (Qt::white, 1.f));
	painter().setBrush (Qt::NoBrush);
	painter().drawRect (block);

	// Filled block showing current value:
	if (_current.valid())
	{
		Angle current = xf::clamped<Angle> (*_current, 0_deg, _maximum);
		QRectF filled_block = block;
		filled_block.setHeight (current / _maximum * filled_block.height());
		painter().setPen (Qt::NoPen);
		painter().setBrush (Qt::white);
		painter().drawRect (filled_block);
	}

	// Target setting in green:
	if (_setting.valid())
	{
		// Green line:
		Angle setting = xf::clamped<Angle> (*_setting, 0_deg, _maximum);
		float w = 0.3f * block.width();
		float s = block.top() + setting / _maximum * block.height();
		painter().setPen (get_pen (Qt::green, 2.f));
		painter().add_shadow ([&] {
			painter().drawLine (QPointF (block.left() - w, s), QPointF (block.right() + w, s));
		});

		// Number or UP
		QString number = "UP";
		if (setting > 0.5_deg)
			number = QString ("%1").arg (xf::symmetric_round (setting.quantity<Degree>()));
		painter().setFont (setting_font);
		painter().fast_draw_text (QPointF (block.right() + 2.f * w, s), Qt::AlignVCenter | Qt::AlignLeft, number);
	}
}

