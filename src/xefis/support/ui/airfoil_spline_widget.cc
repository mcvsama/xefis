/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
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
#include "airfoil_spline_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/painter/arrow.h>
#include <xefis/support/ui/paint_helper.h>
#include <xefis/support/instrument/instrument_aids.h>

// Neutrino:
#include <neutrino/stdexcept.h>
#include <neutrino/qt/painter.h>

// Qt:
#include <QPainter>
#include <QTransform>

// Standard:
#include <cstddef>


namespace xf {

AirfoilSplineWidget::AirfoilSplineWidget (QWidget* parent, Qt::WindowFlags flags):
	CanvasWidget (parent, flags)
{ }


void
AirfoilSplineWidget::set_airfoil (Airfoil const& airfoil)
{
	set_airfoil_spline (airfoil.spline(), airfoil.chord_length());
}


void
AirfoilSplineWidget::set_airfoil_spline (AirfoilSpline const& airfoil_spline, std::optional<si::Length> chord_length)
{
	_airfoil_spline = airfoil_spline;
	_chord_length = chord_length;
	_center_of_pressure_position.reset();
	_lift_force.reset();
	_drag_force.reset();
	_pitching_moment.reset();

	// Calculate center of mass:
	{
		auto const assumed_chord_length = _chord_length.value_or (1_m);
		auto const assumed_wing_length = 1_m;
		auto const assumed_material_density = 1_kg / 1_m3;
		auto const mass_moments = compute_mass_moments_at_arm<AirfoilSplineSpace> (_airfoil_spline, assumed_chord_length, assumed_wing_length, assumed_material_density);
		auto const center_of_mass_3d = mass_moments.center_of_mass_position();
		_center_of_mass_position = { center_of_mass_3d.x() / assumed_chord_length, center_of_mass_3d.y() / assumed_chord_length };
	}

	prepare_for_painting();
	mark_dirty();
}


// QWidget API
void
AirfoilSplineWidget::resizeEvent (QResizeEvent* event)
{
	CanvasWidget::resizeEvent (event);
	setup_painting_transform();
	_pens.reset();
}


void
AirfoilSplineWidget::prepare_for_painting()
{
	auto const& points = _airfoil_spline.points();

	auto const [min_x, max_x] = std::minmax_element (points.begin(), points.end(), [](auto a, auto b) { return a.x() < b.x(); });
	auto const [min_y, max_y] = std::minmax_element (points.begin(), points.end(), [](auto a, auto b) { return a.y() < b.y(); });
	_range[0] = nu::Range (min_x->x(), max_x->x());
	_range[1] = nu::Range (min_y->y(), max_y->y());

	_airfoil_polygon.clear();

	for (auto const& point: points)
		_airfoil_polygon << QPointF (point.x(), point.y());

	if (!points.empty())
		_airfoil_polygon << QPointF (points[0].x(), points[0].y());

	setup_painting_transform();
}


void
AirfoilSplineWidget::setup_painting_transform()
{
	auto const& canvas = this->canvas();
	auto const ph = PaintHelper (canvas, palette(), font());
	_painting_transform.reset();

	// Flip Y to have [0, 0] at bottom left (like in math coordinates):
	_painting_transform.translate (0, canvas.height());
	_painting_transform.scale (1, -1);

	// Calculate margins around the painted airfoil:
	double const default_margin = ph.em_pixels (1.0);
	auto const min_paint_area = QSize (default_margin, default_margin);
	auto const margin_x = 0.5 * (canvas.width() - min_paint_area.width());
	auto const margin_y = 0.5 * (canvas.height() - min_paint_area.height());
	auto const margin = std::min (margin_x, std::min (margin_y, default_margin));

	auto const paint_area_qsize = canvas.size() - 2 * QSizeF (margin, margin);
	double const paint_area[2] = { paint_area_qsize.width(), paint_area_qsize.height() };
	double const paint_area_aspect = paint_area_qsize.width() / paint_area_qsize.height();
	double const airfoil_aspect = _range[0].extent() / _range[1].extent();

	if (paint_area[0] > 0 && paint_area[1] > 0)
	{
		auto x = 0;
		auto y = 1;

		if (airfoil_aspect < paint_area_aspect)
			std::swap (x, y);

		// Calculate origin position and scaling:
		_scale = paint_area[x] / _range[x].extent();
		double const origin[2] = {
			-_range[x].min(),
			0.5 * (paint_area[y] / _scale - _range[y].max() - _range[y].min()),
		};

		// Apply transforms:
		_painting_transform.translate (margin, margin);
		_painting_transform.scale (_scale, _scale);
		_painting_transform.translate (origin[x], origin[y]);
	}
}


void
AirfoilSplineWidget::update_canvas()
{
	auto& canvas = this->canvas();
	canvas.fill (QColor::fromRgb (0xff, 0xfe, 0xf2));

	auto const ph = PaintHelper (canvas, palette(), font());
	auto painter = QPainter (&canvas);
	ph.setup_painter (painter);
	painter.setTransform (_painting_transform);

	if (isVisible())
		update_pens();

	// Draw relative wind direction lines:
	if (_drag_force && _center_of_pressure_position)
	{
		auto const length = (width() + height()) / _scale;
		// Number of arrows to be drawn along each horizontal row:
		auto const num_arrows = 10;
		// Starting (a) and ending (b) points on the horizontal axis for the wind lines:
		auto const a = QPointF (-length, 0);
		auto const b = QPointF (+length, 0);
		// Horizontal step vector (d) between consecutive arrows:
		auto const xstep = (b - a) / num_arrows;
		// Vertical offset (h) for each row of arrows:
		auto const ystep = QPointF (0, ph.em_pixels (2.0) / _scale);
		// Rotation angle for the wind lines based on the drag force vector:
		auto const angle = 1_rad * atan2 (_drag_force->y(), _drag_force->x());

		painter.save();

		painter.translate (_center_of_mass_position);
		painter.rotate (angle.in<si::Degree>());
		painter.setPen (_pens->wind_line_pen);

		// Calculate the number of vertical rows for arrows based on the total length and vertical step:
		auto const num_y = length / ystep.y();

		for (int y = -num_y; y < +num_y; ++y)
		{
			for (int x = 0; x < num_arrows; ++x)
			{
				auto const y_ystep = y * ystep;
				auto const x_xstep = x * xstep;
				draw_arrow (painter, a + x_xstep + y_ystep, a + (x_xstep + xstep) + y_ystep);
			}
		}

		painter.restore();
	}

	// Airfoil:
	{
		painter.setPen (_pens->airfoil_pen);
		painter.setBrush (_pens->airfoil_brush);
		painter.drawPolygon (_airfoil_polygon);
	}

	// Coordinate lines:
	{
		painter.setPen (_pens->coordinate_lines_pen);
		painter.drawLine (QPoint (-2, 0), QPoint (+2, 0));
		painter.drawLine (QPoint (0, -2), QPoint (0, +2));
	}

	// Center of mass:
	{
		using namespace nu::painter_literals;

		auto const r = ph.em_pixels (0.5) / _scale;
		painter.save();
		painter.translate (_center_of_mass_position);
		painter.setPen (_pens->center_of_mass_black_pen);
		painter.setBrush (Qt::black);
		painter.drawPie (QRectF (QPointF (-r, +r), QPointF (+r, -r)), 0_qarcdeg, -90_qarcdeg);
		painter.drawPie (QRectF (QPointF (-r, +r), QPointF (+r, -r)), -180_qarcdeg, -90_qarcdeg);
		painter.setBrush (Qt::white);
		painter.drawPie (QRectF (QPointF (-r, +r), QPointF (+r, -r)), -90_qarcdeg, -90_qarcdeg);
		painter.drawPie (QRectF (QPointF (-r, +r), QPointF (+r, -r)), -270_qarcdeg, -90_qarcdeg);
		painter.restore();
	}

	// Lift force:
	if (_lift_force && _center_of_pressure_position)
	{
		painter.save();

		if (_center_of_pressure_position_relative_to_com)
			painter.translate (_center_of_mass_position);

		painter.translate (*_center_of_pressure_position);
		painter.setPen (_pens->lift_force_pen);
		painter.setBrush (Qt::green);
		auto const lift_force_unitless = *_lift_force / _force_per_spline_space_unit;
		draw_arrow (painter, QPointF (0.0f, 0.0f), QPointF (lift_force_unitless.x(), lift_force_unitless.y()));
		painter.restore();
	}

	// Drag force:
	if (_drag_force && _center_of_pressure_position)
	{
		painter.save();

		if (_center_of_pressure_position_relative_to_com)
			painter.translate (_center_of_mass_position);

		painter.translate (*_center_of_pressure_position);
		painter.setPen (_pens->drag_force_pen);
		painter.setBrush (Qt::red);
		auto const drag_force_unitless = *_drag_force / _force_per_spline_space_unit;
		draw_arrow (painter, QPointF (0.0f, 0.0f), QPointF (drag_force_unitless.x(), drag_force_unitless.y()));
		painter.restore();
	}

	// Center of pressure:
	if (_center_of_pressure_position)
	{
		auto const r = ph.em_pixels (0.25) / _scale;
		painter.save();

		if (_center_of_pressure_position_relative_to_com)
			painter.translate (_center_of_mass_position);

		painter.translate (*_center_of_pressure_position);
		painter.setPen (_pens->center_of_pressure_pen);
		painter.setBrush (Qt::white);
		painter.drawEllipse (QRectF (QPointF (-r, +r), QPointF (+r, -r)));
		painter.restore();
	}

	// TODO visualize pitching moment
	// TODO visualize angle of attack: beta?
}


void
AirfoilSplineWidget::update_pens()
{
	if (!_pens)
	{
		auto const ph = PaintHelper (this->canvas(), palette(), font());
		auto const airfoil_color = QColor (0xd2, 0xc3, 0xb1, 0xff);

		_pens = {
			.coordinate_lines_pen		= QPen (Qt::black, ph.em_pixels (0.05f) / _scale, Qt::SolidLine, Qt::RoundCap),
			.airfoil_pen				= QPen (airfoil_color.darker (150), ph.em_pixels (0.05f) / _scale, Qt::SolidLine, Qt::RoundCap),
			.airfoil_brush				= QBrush (airfoil_color),
			.center_of_mass_black_pen	= QPen (Qt::black, ph.em_pixels (0.1f) / _scale, Qt::SolidLine, Qt::FlatCap),
			.lift_force_pen				= QPen (Qt::green, ph.em_pixels (0.1f) / _scale, Qt::SolidLine, Qt::RoundCap),
			.drag_force_pen				= QPen (Qt::red, ph.em_pixels (0.1f) / _scale, Qt::SolidLine, Qt::RoundCap),
			.center_of_pressure_pen		= QPen (Qt::blue, ph.em_pixels (0.1f) / _scale, Qt::SolidLine, Qt::FlatCap),
			.wind_line_pen				= QPen (Qt::gray, ph.em_pixels (0.05f) / _scale, Qt::SolidLine, Qt::FlatCap),
		};
	}
}

} // namespace xf

