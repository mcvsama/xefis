/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
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
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "imu_cff.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/imu-cff", IMUCFF);


IMUCFF::IMUCFF (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	Time smoothing_time = 0_s;

	parse_settings (config, {
		{ "smoothing-time", smoothing_time, false },
	});

	parse_properties (config, {
		// Input:
		{ "input.rotation.x", _rotation_x, true },
		{ "input.rotation.y", _rotation_y, true },
		{ "input.rotation.z", _rotation_z, true },
		{ "input.ias.x", _ias_x, true },
		{ "input.ias.y", _ias_y, true },
		{ "input.ias.z", _ias_z, true },
		// Output:
		{ "output.centrifugal-accel.x", _centrifugal_x, true },
		{ "output.centrifugal-accel.y", _centrifugal_y, true },
		{ "output.centrifugal-accel.z", _centrifugal_z, true },
	});

	_smooth_cf_y.set_smoothing_time (smoothing_time);
	_smooth_cf_z.set_smoothing_time (smoothing_time);

	_centrifugal_computer.set_callback (std::bind (&IMUCFF::compute_centrifugal, this));
	_centrifugal_computer.observe ({
		&_rotation_x,
		&_rotation_y,
		&_rotation_z,
		&_ias_x,
		&_ias_y,
		&_ias_z,
	});
}


void
IMUCFF::data_updated()
{
	_centrifugal_computer.data_updated (update_time());
}


void
IMUCFF::compute_centrifugal()
{
	if (_ias_x.valid())
	{
		Time dt = _centrifugal_computer.update_dt();

		/*
		 * Turn radius:
		 *   r = v / (2 * pi * f)
		 * r <- radius
		 * f <- frequency
		 * v <- ias
		 *   a = v^2 / r
		 * Therefore:
		 *   a = v * 2 * pi * f
		 * And:
		 *   a[y] = v[x] * (2 * pi * -f[z])
		 *   a[z] = v[x] * (2 * pi * +f[y])
		 */

		// X is not computed:
		_centrifugal_x.set_nil();

		if (_rotation_z.valid())
			_centrifugal_y = 1_mps2 * _smooth_cf_y.process ((*_ias_x * (2.0 * M_PI * -*_rotation_z)).mps2(), dt);
		else
			_centrifugal_y.set_nil();

		if (_rotation_y.valid())
			_centrifugal_z = 1_mps2 * _smooth_cf_z.process ((*_ias_x * (2.0 * M_PI * *_rotation_y)).mps2(), dt);
		else
			_centrifugal_z.set_nil();
	}
	else
	{
		_centrifugal_x.set_nil();
		_centrifugal_y.set_nil();
		_centrifugal_z.set_nil();
	}
}

