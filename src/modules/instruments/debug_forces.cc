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


XEFIS_REGISTER_MODULE_CLASS ("instruments/debug-forces", DebugForces)


DebugForces::DebugForces (v1::ModuleManager* module_manager, QDomElement const& config):
	Instrument (module_manager, config),
	InstrumentAids (1.0)
{
	parse_properties (config, {
		{ "input.orientation.pitch", _input_orientation_pitch, true },
		{ "input.orientation.roll", _input_orientation_roll, true },
		{ "input.orientation.heading.magnetic", _input_orientation_magnetic_heading, true },
		{ "input.accel.measured.x", _input_measured_accel_x, true },
		{ "input.accel.measured.y", _input_measured_accel_y, true },
		{ "input.accel.measured.z", _input_measured_accel_z, true },
		{ "input.accel.centrifugal.x", _input_centrifugal_accel_x, true },
		{ "input.accel.centrifugal.y", _input_centrifugal_accel_y, true },
		{ "input.accel.centrifugal.z", _input_centrifugal_accel_z, true },
	});

	update();
}


void
DebugForces::data_updated()
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

	if (_input_centrifugal_accel_y.valid() && _input_centrifugal_accel_z.valid())
		centrifugal_accel = QPointF (_input_centrifugal_accel_y->quantity<Gravity>() * one_gravity_length,
									 _input_centrifugal_accel_z->quantity<Gravity>() * one_gravity_length);

	if (_input_measured_accel_y.valid() && _input_measured_accel_z.valid())
		measured_accel = QPointF (_input_measured_accel_y->quantity<Gravity>() * one_gravity_length,
								  _input_measured_accel_z->quantity<Gravity>() * one_gravity_length);

	earth_accel = measured_accel - centrifugal_accel;

	painter().translate (0.5 * width(), 0.5 * height());

	// Horizon reference frame:
	painter().setPen (get_pen (Qt::white, 0.5));
	painter().drawLine (QPointF (-0.5 * width(), 0.0), QPointF (0.5 * width(), 0.0));

	if (_input_orientation_roll.valid())
	{
		// Plane reference frame:
		painter().rotate (_input_orientation_roll->quantity<Degree>());
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
}

