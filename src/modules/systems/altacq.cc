/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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
		{ "flag-altitude-difference.on", _flag_diff_on, false },
		{ "flag-altitude-difference.off", _flag_diff_off, false },
	});

	parse_properties (config, {
		// Input:
		{ "altitude.amsl", _altitude_amsl, true },
		{ "altitude.acquire.amsl", _altitude_acquire_amsl, true },
		{ "vertical-speed", _vertical_speed, false },
		{ "ground-speed", _ground_speed, false },
		// Output:
		{ "altitude.acquire.distance", _altitude_acquire_distance, false },
		{ "altitude.acquire.flag", _altitude_acquire_flag, false },
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

	if (_altitude_acquire_flag.configured() && _altitude_amsl.valid() && _altitude_acquire_amsl.valid())
	{
		if (_altitude_amsl.fresh() || _altitude_acquire_amsl.fresh())
		{
			if (_altitude_acquire_amsl.fresh())
				_altitude_acquire_amsl_timestamp = Time::now();

			Length diff = 1_ft * std::abs ((*_altitude_amsl - *_altitude_acquire_amsl).ft());
			// Arm flag when difference beyond 'on-diff':
			if (diff > _flag_diff_on)
				_flag_armed = true;
			// But don't allow arming if alt setting was changed recently:
			if (Time::now() - _altitude_acquire_amsl_timestamp < 1_s)
				_flag_armed = false;
			// Disarm and disable when approaching commanded altitude,
			// so that it doesn't engage again when the craft is on the
			// other side of commanded alt:
			if (diff < _flag_diff_off)
				_flag_armed = false;

			_altitude_acquire_flag.write (_flag_armed && _flag_diff_off <= diff && diff <= _flag_diff_on);
		}
	}
	else
		_altitude_acquire_flag.set_nil();
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

