/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include <xefis/config/exception.h>
#include <xefis/utility/qdom.h>

// Local:
#include "altacq.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/altacq", AltAcq);


AltAcq::AltAcq (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "minimum-altitude-difference", _minimum_altitude_difference, false },
	});

	parse_properties (config, {
		// Input:
		{ "altitude.amsl", _altitude_amsl, false },
		{ "altitude.acquire.amsl", _altitude_acquire_amsl, false },
		{ "vertical-speed", _vertical_speed, false },
		{ "ground-speed", _ground_speed, false },
		// Output:
		{ "altitude.acquire.distance", _altitude_acquire_distance, true },
	});

	_altitude_acquire_distance_computer.set_minimum_dt (100_ms);
	_altitude_acquire_distance_computer.set_callback (std::bind (&AltAcq::compute_altitude_acquire_distance, this));
	_altitude_acquire_distance_computer.add_depending_smoothers ({
		&_altitude_acquire_distance_smoother,
	});
	_altitude_acquire_distance_computer.observe ({
		&_altitude_acquire_amsl,
		&_altitude_amsl,
		&_vertical_speed,
		&_ground_speed,
	});
}


void
AltAcq::data_updated()
{
	_altitude_acquire_distance_computer.data_updated (update_time());
}


void
AltAcq::compute_altitude_acquire_distance()
{
	Time update_dt = _altitude_acquire_distance_computer.update_dt();

	if (_altitude_acquire_amsl.valid() &&
		_altitude_amsl.valid() &&
		_vertical_speed.valid() &&
		_ground_speed.valid())
	{
		Length const alt_diff = *_altitude_acquire_amsl - *_altitude_amsl;
		Length const distance = *_ground_speed * (alt_diff / *_vertical_speed);

		if (!has_setting ("minimum-altitude-difference") || std::abs (alt_diff.ft()) >= _minimum_altitude_difference.ft())
			_altitude_acquire_distance.write (1_m * _altitude_acquire_distance_smoother.process (distance.m(), update_dt));
		else
			_altitude_acquire_distance.set_nil();
	}
	else
	{
		_altitude_acquire_distance.set_nil();
		_altitude_acquire_distance_smoother.invalidate();
	}
}

