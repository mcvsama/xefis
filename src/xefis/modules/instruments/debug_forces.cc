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
#include "debug_forces.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


DebugForces::DebugForces (xf::Graphics const& graphics, std::string_view const& instance):
	DebugForcesIO (instance),
	InstrumentSupport (graphics)
{ }


void
DebugForces::process (xf::Cycle const&)
{
	mark_dirty();
}


std::packaged_task<void()>
DebugForces::paint (xf::PaintRequest paint_request) const
{
	PaintingParams params;
	params.orientation_pitch = _io.orientation_pitch.get_optional();
	params.orientation_roll = _io.orientation_roll.get_optional();
	params.orientation_magnetic_heading = _io.orientation_magnetic_heading.get_optional();
	params.measured_accel_x = _io.measured_accel_x.get_optional();
	params.measured_accel_y = _io.measured_accel_y.get_optional();
	params.measured_accel_z = _io.measured_accel_z.get_optional();
	params.centrifugal_accel_x = _io.centrifugal_accel_x.get_optional();
	params.centrifugal_accel_y = _io.centrifugal_accel_y.get_optional();
	params.centrifugal_accel_z = _io.centrifugal_accel_z.get_optional();

	return std::packaged_task<void()> ([this, pr = std::move (paint_request), pp = std::move (params)] {
		async_paint (pr, pp);
	});
}


void
DebugForces::async_paint (xf::PaintRequest const& paint_request, PaintingParams const& pp) const
{
	auto aids = get_aids (paint_request);
	auto painter = get_painter (paint_request);
	painter.fillRect (paint_request.metric().canvas_rect(), QColor (0x55, 0x63, 0x71));

	double one_gravity_length = 0.15 * aids->height();
	QPointF center (0.0, 0.0);
	QPointF centrifugal_accel;
	QPointF measured_accel;
	QPointF earth_accel;

	if (pp.centrifugal_accel_y && pp.centrifugal_accel_z)
		centrifugal_accel = QPointF (pp.centrifugal_accel_y->in<si::Gravity>() * one_gravity_length,
									 pp.centrifugal_accel_z->in<si::Gravity>() * one_gravity_length);

	if (pp.measured_accel_y && pp.measured_accel_z)
		measured_accel = QPointF (pp.measured_accel_y->in<si::Gravity>() * one_gravity_length,
								  pp.measured_accel_z->in<si::Gravity>() * one_gravity_length);

	earth_accel = measured_accel - centrifugal_accel;

	painter.translate (0.5 * aids->width(), 0.5 * aids->height());

	// Horizon reference frame:
	painter.setPen (aids->get_pen (Qt::white, 0.5));
	painter.drawLine (QPointF (-0.5 * aids->width(), 0.0), QPointF (0.5 * aids->width(), 0.0));

	if (pp.orientation_roll)
	{
		// Plane reference frame:
		painter.rotate (pp.orientation_roll->in<si::Degree>());
		// Plane:
		painter.setPen (aids->get_pen (Qt::white, 2.5));
		painter.drawLine (QPointF (-0.25 * aids->width(), 0.0), QPointF (0.25 * aids->width(), 0.0));
		// Real acceleration:
		painter.setPen (aids->get_pen (Qt::yellow, 1.0));
		painter.drawLine (center, earth_accel);
		// Measured acceleration:
		painter.setPen (aids->get_pen (Qt::red, 1.0));
		painter.drawLine (center, measured_accel);
		// Centrifugal acceleration:
		painter.setPen (aids->get_pen (Qt::blue, 1.0));
		painter.drawLine (center, centrifugal_accel);
	}

	// TODO set output sockets
}

