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

// Local:
#include "debug_forces.h"


DebugForces::DebugForces (std::unique_ptr<DebugForcesIO> module_io, xf::Graphics const& graphics, std::string_view const& instance):
	Instrument (std::move (module_io), instance),
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
	return std::packaged_task<void()> ([&, pr = std::move (paint_request)] {
		async_paint (pr);
	});
}


void
DebugForces::async_paint (xf::PaintRequest const& paint_request) const
{
	auto aids = get_aids (paint_request);
	auto painter = get_painter (paint_request);
	paint_request.canvas().fill (QColor (0x55, 0x63, 0x71));

	double one_gravity_length = 0.15 * aids->height();
	QPointF center (0.0, 0.0);
	QPointF centrifugal_accel;
	QPointF measured_accel;
	QPointF earth_accel;

	if (io.centrifugal_accel_y && io.centrifugal_accel_z)
		centrifugal_accel = QPointF (io.centrifugal_accel_y->in<Gravity>() * one_gravity_length,
									 io.centrifugal_accel_z->in<Gravity>() * one_gravity_length);

	if (io.measured_accel_y && io.measured_accel_z)
		measured_accel = QPointF (io.measured_accel_y->in<Gravity>() * one_gravity_length,
								  io.measured_accel_z->in<Gravity>() * one_gravity_length);

	earth_accel = measured_accel - centrifugal_accel;

	painter.translate (0.5 * aids->width(), 0.5 * aids->height());

	// Horizon reference frame:
	painter.setPen (aids->get_pen (Qt::white, 0.5));
	painter.drawLine (QPointF (-0.5 * aids->width(), 0.0), QPointF (0.5 * aids->width(), 0.0));

	if (io.orientation_roll)
	{
		// Plane reference frame:
		painter.rotate (io.orientation_roll->in<Degree>());
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

	// TODO set output properties
}

