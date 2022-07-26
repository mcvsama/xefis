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
#include "altacq.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/time_helper.h>

// Standard:
#include <cstddef>


AltAcq::AltAcq (std::string_view const& instance):
	AltAcqIO (instance)
{
	_output_computer.set_minimum_dt (100_ms);
	_output_computer.set_callback (std::bind (&AltAcq::compute_altitude_acquire_distance, this));
	_output_computer.add_depending_smoothers ({
		&_output_smoother,
	});
	_output_computer.observe ({
		&_io.altitude_acquire_amsl,
		&_io.altitude_amsl,
		&_io.vertical_speed,
		&_io.ground_speed,
	});
}


void
AltAcq::process (xf::Cycle const& cycle)
{
	_output_computer.process (cycle.update_time());

	if (_io.altitude_acquire_flag.use_count() > 0 && _io.altitude_amsl && _io.altitude_acquire_amsl)
	{
		if (_altitude_amsl_changed.value_changed() || _altitude_acquire_amsl_changed.value_changed())
		{
			si::Length diff = abs (*_io.altitude_amsl - *_io.altitude_acquire_amsl);
			// Arm flag when difference beyond 'on-diff':
			if (diff > *_io.flag_diff_on)
				_flag_armed = true;
			// But don't allow arming if alt setting was changed recently:
			if (_io.altitude_acquire_amsl.valid_age() < 1_s)
				_flag_armed = false;
			// Disarm and disable when approaching commanded altitude,
			// so that it doesn't engage again when the craft is on the
			// other side of commanded alt:
			if (diff < *_io.flag_diff_off)
				_flag_armed = false;

			_io.altitude_acquire_flag = _flag_armed && *_io.flag_diff_off <= diff && diff <= *_io.flag_diff_on;
		}
	}
	else
		_io.altitude_acquire_flag = xf::nil;
}


void
AltAcq::compute_altitude_acquire_distance()
{
	si::Time update_dt = _output_computer.update_dt();

	if (_io.altitude_acquire_amsl &&
		_io.altitude_amsl &&
		_io.vertical_speed &&
		_io.ground_speed)
	{
		si::Length const alt_diff = *_io.altitude_acquire_amsl - *_io.altitude_amsl;
		si::Length const distance = *_io.ground_speed * (alt_diff / *_io.vertical_speed);

		if (!_io.minimum_altitude_difference || abs (alt_diff) >= *_io.minimum_altitude_difference)
			_io.altitude_acquire_distance = _output_smoother (distance, update_dt);
		else
			_io.altitude_acquire_distance = xf::nil;
	}
	else
	{
		_io.altitude_acquire_distance = xf::nil;
		_output_smoother.invalidate();
	}
}

