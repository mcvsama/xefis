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
#include <xefis/utility/time_helper.h>

// Local:
#include "altacq.h"


AltAcq::AltAcq (std::string const& instance):
	Module (instance)
{
	_output_computer.set_minimum_dt (100_ms);
	_output_computer.set_callback (std::bind (&AltAcq::compute_altitude_acquire_distance, this));
	_output_computer.add_depending_smoothers ({
		&_output_smoother,
	});
	_output_computer.observe ({
		&input_altitude_acquire_amsl,
		&input_altitude_amsl,
		&input_vertical_speed,
		&input_ground_speed,
	});
}


void
AltAcq::process (x2::Cycle const& cycle)
{
	_output_computer.process (cycle.update_time());

	if (output_altitude_acquire_flag.connected() && input_altitude_amsl && input_altitude_acquire_amsl)
	{
		if (_altitude_amsl_changed() || _altitude_acquire_amsl_changed())
		{
			si::Length diff = abs (*input_altitude_amsl - *input_altitude_acquire_amsl);
			// Arm flag when difference beyond 'on-diff':
			if (diff > *setting_flag_diff_on)
				_flag_armed = true;
			// But don't allow arming if alt setting was changed recently:
			if (input_altitude_acquire_amsl.valid_age() < 1_s)
				_flag_armed = false;
			// Disarm and disable when approaching commanded altitude,
			// so that it doesn't engage again when the craft is on the
			// other side of commanded alt:
			if (diff < *setting_flag_diff_off)
				_flag_armed = false;

			output_altitude_acquire_flag = _flag_armed && *setting_flag_diff_off <= diff && diff <= *setting_flag_diff_on;
		}
	}
	else
		output_altitude_acquire_flag.set_nil();
}


void
AltAcq::compute_altitude_acquire_distance()
{
	si::Time update_dt = _output_computer.update_dt();

	if (input_altitude_acquire_amsl &&
		input_altitude_amsl &&
		input_vertical_speed &&
		input_ground_speed)
	{
		si::Length const alt_diff = *input_altitude_acquire_amsl - *input_altitude_amsl;
		si::Length const distance = *input_ground_speed * (alt_diff / *input_vertical_speed);

		if (!setting_minimum_altitude_difference || abs (alt_diff) >= *setting_minimum_altitude_difference)
			output_altitude_acquire_distance = _output_smoother (distance, update_dt);
		else
			output_altitude_acquire_distance.set_nil();
	}
	else
	{
		output_altitude_acquire_distance.set_nil();
		_output_smoother.invalidate();
	}
}

