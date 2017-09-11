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


DebugForces::DebugForces (std::unique_ptr<DebugForcesIO> module_io, std::string const& instance):
	Instrument (std::move (module_io), instance),
	InstrumentAids (1.0f)
{ }


void
DebugForces::process (v2::Cycle const&)
{
	update();
}


void
DebugForces::paintEvent (QPaintEvent*)
{
	auto painting_token = get_token (this);
	clear_background (QColor (0x55, 0x63, 0x71));

	double one_gravity_length = 0.15 * height();
	QPointF center (0.0, 0.0);
	QPointF centrifugal_accel;
	QPointF measured_accel;
	QPointF earth_accel;

	if (io.centrifugal_accel_y && io.centrifugal_accel_z)
		centrifugal_accel = QPointF (io.centrifugal_accel_y->quantity<Gravity>() * one_gravity_length,
									 io.centrifugal_accel_z->quantity<Gravity>() * one_gravity_length);

	if (io.measured_accel_y && io.measured_accel_z)
		measured_accel = QPointF (io.measured_accel_y->quantity<Gravity>() * one_gravity_length,
								  io.measured_accel_z->quantity<Gravity>() * one_gravity_length);

	earth_accel = measured_accel - centrifugal_accel;

	painter().translate (0.5 * width(), 0.5 * height());

	// Horizon reference frame:
	painter().setPen (get_pen (Qt::white, 0.5));
	painter().drawLine (QPointF (-0.5 * width(), 0.0), QPointF (0.5 * width(), 0.0));

	if (io.orientation_roll)
	{
		// Plane reference frame:
		painter().rotate (io.orientation_roll->quantity<Degree>());
		// Plane:
		painter().setPen (get_pen (Qt::white, 2.5));
		painter().drawLine (QPointF (-0.25 * width(), 0.0), QPointF (0.25 * width(), 0.0));
		// Real acceleration:
		painter().setPen (get_pen (Qt::yellow, 1.0));
		painter().drawLine (center, earth_accel);
		// Measured acceleration:
		painter().setPen (get_pen (Qt::red, 1.0));
		painter().drawLine (center, measured_accel);
		// Centrifugal acceleration:
		painter().setPen (get_pen (Qt::blue, 1.0));
		painter().drawLine (center, centrifugal_accel);
	}

	// TODO set output properties
}

