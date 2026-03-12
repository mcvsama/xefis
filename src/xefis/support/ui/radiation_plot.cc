/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
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
#include "radiation_plot.h"
#include "widget_utils.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/ui/paint_helper.h>

// Neutrino:
#include <neutrino/qt/painter.h>
#include <neutrino/qt/qpalette.h>

// Qt:
#include <QPainter>
#include <QString>

// Standard:
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <numbers>


namespace xf {

RadiationPlot::RadiationPlot (QWidget* parent, Qt::WindowFlags flags):
	CanvasWidget (parent, flags)
{ }


void
RadiationPlot::set_antenna_model (AntennaModel const& antenna_model)
{
	_antenna_model = &antenna_model;
	prepare_for_painting();
	mark_dirty();
}


void
RadiationPlot::set_axis_vector (SpaceVector<double, BodyOrigin> const& axis_vector)
{
	_axis_vector = axis_vector;
	prepare_for_painting();
	mark_dirty();
}


void
RadiationPlot::set_num_samples (std::size_t num_samples)
{
	num_samples = std::clamp (num_samples, 12uz, 4096uz);

	if (_num_samples != num_samples)
	{
		_num_samples = num_samples;
		prepare_for_painting();
		mark_dirty();
	}
}


void
RadiationPlot::resizeEvent (QResizeEvent* event)
{
	CanvasWidget::resizeEvent (event);
	_pens.reset();
}


void
RadiationPlot::changeEvent (QEvent* event)
{
	CanvasWidget::changeEvent (event);

	if (nu::is_theme_change (event))
		_pens.reset();
}


void
RadiationPlot::prepare_for_painting()
{
	_radiation_pattern_polygon.clear();
	_horizontal_positive_axis_label.clear();
	_horizontal_negative_axis_label.clear();
	_vertical_positive_axis_label.clear();
	_vertical_negative_axis_label.clear();
	_view_axis_label.clear();

	if (_antenna_model)
	{
		auto view_axis = normalized_direction_or_zero (_axis_vector);

		if (abs (view_axis) <= 1e-9)
			view_axis = { 0.0, 0.0, 1.0 };

		// Build a stable in-plane basis: vertical first, then horizontal.
		auto vertical_axis = SpaceVector<double, BodyOrigin> { 0.0, 0.0, 1.0 };
		vertical_axis -= dot_product (vertical_axis, view_axis) * view_axis;
		vertical_axis = normalized_direction_or_zero (vertical_axis);

		if (abs (vertical_axis) <= 1e-9)
		{
			vertical_axis = SpaceVector<double, BodyOrigin> { 0.0, 1.0, 0.0 };
			vertical_axis -= dot_product (vertical_axis, view_axis) * view_axis;
			vertical_axis = normalized_direction_or_zero (vertical_axis);
		}

		if (abs (vertical_axis) > 1e-9)
		{
			auto horizontal_axis = normalized_direction_or_zero (cross_product (vertical_axis, view_axis));

			if (abs (horizontal_axis) > 1e-9)
			{
				_horizontal_positive_axis_label = axis_label_for (horizontal_axis);
				_horizontal_negative_axis_label = axis_label_for (-horizontal_axis);
				_vertical_positive_axis_label = axis_label_for (vertical_axis);
				_vertical_negative_axis_label = axis_label_for (-vertical_axis);
				_view_axis_label = QString ("View %1").arg (axis_label_for (view_axis));

				for (std::size_t i = 0; i <= _num_samples; ++i)
				{
					auto const angle = 2.0 * std::numbers::pi * double (i) / double (_num_samples);
					auto const cosine = std::cos (angle);
					auto const sine = std::sin (angle);
					auto const direction = cosine * horizontal_axis + sine * vertical_axis;
					auto const pattern = std::clamp (_antenna_model->radiation_pattern (direction), 0.0, 1.0);

					_radiation_pattern_polygon << QPointF (pattern * cosine, pattern * sine);
				}
			}
		}
	}
}


void
RadiationPlot::update_canvas()
{
	if (!isVisible())
		return;

	auto& canvas = this->canvas();
	auto const color_group = widget_color_group (*this);
	auto const colors = theme_colors_for (palette(), color_group);
	canvas.fill (colors.background);

	auto const ph = PaintHelper (canvas, palette(), font());
	auto painter = QPainter (&canvas);
	ph.setup_painter (painter);
	painter.setRenderHint (QPainter::Antialiasing);

	auto const margin = ph.em_pixels (1.0);
	auto const radius = 0.5 * std::min (canvas.width(), canvas.height()) - margin;

	if (radius > 1.0)
	{
		auto const center = QPointF (0.5 * canvas.width(), 0.5 * canvas.height());

		update_pens();

		painter.setPen (_pens->coordinate_circles_pen);
		painter.setBrush (Qt::NoBrush);

		for (auto const scale: std::array { 1.0, 0.75, 0.5, 0.25 })
			painter.drawEllipse (center, radius * scale, radius * scale);

		painter.setPen (_pens->coordinate_lines_pen);
		painter.drawLine (QPointF (center.x() - radius, center.y()), QPointF (center.x() + radius, center.y()));
		painter.drawLine (QPointF (center.x(), center.y() - radius), QPointF (center.x(), center.y() + radius));

		if (!_horizontal_positive_axis_label.isEmpty() && !_horizontal_negative_axis_label.isEmpty() &&
			!_vertical_positive_axis_label.isEmpty() && !_vertical_negative_axis_label.isEmpty() && !_view_axis_label.isEmpty())
		{
			auto axis_font = painter.font();
			axis_font.setPixelSize (std::max (1, int (ph.em_pixels (0.8))));
			painter.setFont (axis_font);

			auto const label_width = ph.em_pixels (1.5);
			auto const label_height = ph.em_pixels (1);
			auto const dw = 0.5 * label_width;
			auto const dh = 0.5 * label_height;
			auto const template_rect = QRectF (QPointF (0.0, 0.0), QSizeF (label_width, label_height));

			auto horiz_pos = template_rect;
			horiz_pos.moveCenter (QPointF (center.x() + radius + dw, center.y()));

			auto horiz_neg = template_rect;
			horiz_neg.moveCenter (QPointF (center.x() - radius - dw, center.y()));

			auto vert_pos = template_rect;
			vert_pos.moveCenter (QPointF (center.x(), center.y() - radius - dh));

			auto vert_neg = template_rect;
			vert_neg.moveCenter (QPointF (center.x(), center.y() + radius + dh));

			painter.setPen (_pens->coordinate_lines_pen);
			painter.drawText(horiz_pos, Qt::AlignHCenter | Qt::AlignVCenter, _horizontal_positive_axis_label);
			painter.drawText(horiz_neg, Qt::AlignHCenter | Qt::AlignVCenter, _horizontal_negative_axis_label);
			painter.drawText(vert_pos, Qt::AlignHCenter | Qt::AlignVCenter, _vertical_positive_axis_label);
			painter.drawText(vert_neg, Qt::AlignHCenter | Qt::AlignVCenter, _vertical_negative_axis_label);
		}

		if (!_radiation_pattern_polygon.isEmpty())
		{
			QPolygonF polygon;
			polygon.reserve (_radiation_pattern_polygon.size());

			for (auto const& point: _radiation_pattern_polygon)
				polygon << QPointF (center.x() + radius * point.x(), center.y() - radius * point.y());

			painter.setPen (_pens->radiation_pattern_pen);
			painter.setBrush (_pens->radiation_pattern_brush);
			painter.drawPolygon (polygon);
		}

		painter.setPen (_pens->coordinate_lines_pen);
		painter.setBrush (Qt::NoBrush);
		painter.drawEllipse (center, radius, radius);
	}
}


void
RadiationPlot::update_pens()
{
	if (!_pens)
	{
		auto const ph = PaintHelper (this->canvas(), palette(), font());
		auto const color_group = widget_color_group (*this);
		auto const colors = theme_colors_for (palette(), color_group);

		_pens = {
			.coordinate_lines_pen		= QPen (colors.coordinate_lines, ph.em_pixels (0.08f), Qt::SolidLine, Qt::FlatCap),
			.coordinate_circles_pen		= QPen (colors.coordinate_circles, ph.em_pixels (0.04f), Qt::SolidLine, Qt::FlatCap),
			.radiation_pattern_pen		= QPen (colors.pattern_outline, ph.em_pixels (0.08f), Qt::SolidLine, Qt::RoundCap),
			.radiation_pattern_brush	= QBrush (colors.pattern_fill),
		};
	}
}


RadiationPlot::ThemeColors
RadiationPlot::theme_colors_for (QPalette const& palette, QPalette::ColorGroup color_group)
{
	// Keep the same warm paper feel as other engineering plots.
	if (nu::is_light_theme (palette))
	{
		return {
			.background				= QColor::fromRgb (0xff, 0xfe, 0xf2),
			.coordinate_lines		= QColor::fromRgb (0x34, 0x34, 0x34),
			.coordinate_circles		= QColor::fromRgb (0x7d, 0x7d, 0x7d),
			.pattern_fill			= QColor::fromRgb (0x74, 0xa7, 0x3c, 0x78),
			.pattern_outline		= QColor::fromRgb (0x3f, 0x6f, 0x21),
		};
	}
	else
	{
		return {
			.background				= QColor::fromRgb (0x2d, 0x28, 0x23),
			.coordinate_lines		= palette.color (color_group, QPalette::WindowText),
			.coordinate_circles		= QColor::fromRgb (0x92, 0x88, 0x78),
			.pattern_fill			= QColor::fromRgb (0x93, 0xbe, 0x5f, 0x58),
			.pattern_outline		= QColor::fromRgb (0xb7, 0xd8, 0x86),
		};
	}
}


QString
RadiationPlot::axis_label_for (SpaceVector<double, BodyOrigin> const& direction)
{
	auto const normalized_direction = normalized_direction_or_zero (direction);

	if (abs (normalized_direction) <= 1e-9)
		return QString();

	auto const abs_direction = std::array {
		std::abs (normalized_direction[0]),
		std::abs (normalized_direction[1]),
		std::abs (normalized_direction[2]),
	};
	auto const max_iterator = std::max_element (abs_direction.begin(), abs_direction.end());
	auto const axis_index = std::size_t (std::distance (abs_direction.begin(), max_iterator));
	auto const sign = normalized_direction[axis_index] < 0.0 ? '-' : '+';
	auto const axis_name = std::array<QChar, 3> { 'X', 'Y', 'Z' }[axis_index];
	return QString ("%1%2").arg (sign).arg (axis_name);
}

} // namespace xf
