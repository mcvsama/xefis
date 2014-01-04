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
#include <xefis/utility/qdom.h>

// Local:
#include "fly_by_wire.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/fly-by-wire", FlyByWire);


FlyByWire::FlyByWire (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config),
	_manual_pitch_pid (1.0, 0.1, 0.0, 0.0),
	_manual_roll_pid (1.0, 0.1, 0.0, 0.0),
	_elevator_pid (0.0, 0.0, 0.0, 0.0),
	_ailerons_pid (0.0, 0.0, 0.0, 0.0),
	_rudder_pid (0.0, 0.0, 0.0, 0.0)
{
	parse_settings (config, {
		{ "stabilization-gain", _stabilization_gain, true },
		{ "pitch-gain", _pitch_gain, true },
		{ "pitch-p", _pitch_p, true },
		{ "pitch-i", _pitch_i, true },
		{ "pitch-d", _pitch_d, true },
		{ "pitch-error-power", _pitch_error_power, true },
		{ "roll-gain", _roll_gain, true },
		{ "roll-p", _roll_p, true },
		{ "roll-i", _roll_i, true },
		{ "roll-d", _roll_d, true },
		{ "roll-error-power", _roll_error_power, true },
		{ "yaw-gain", _yaw_gain, true },
		{ "yaw-p", _yaw_p, true },
		{ "yaw-i", _yaw_i, true },
		{ "yaw-d", _yaw_d, true },
		{ "yaw-error-power", _yaw_error_power, true },
	});

	parse_properties (config, {
		{ "attitude-mode", _attitude_mode, true },
		{ "throttle-mode", _throttle_mode, true },
		{ "input.pitch-axis", _input_pitch_axis, true },
		{ "input.roll-axis", _input_roll_axis, true },
		{ "input.yaw-axis", _input_yaw_axis, true },
		{ "input.throttle-axis", _input_throttle_axis, true },
		{ "pitch-extent", _pitch_extent, true },
		{ "roll-extent", _roll_extent, true },
		{ "input.pitch", _input_pitch, true },
		{ "input.roll", _input_roll, true },
		{ "input.throttle", _input_throttle, true },
		{ "measured.pitch", _measured_pitch, true },
		{ "measured.roll", _measured_roll, true },
		{ "measured.slip-skid", _measured_slip_skid_g, true },
		{ "elevator.minimum", _elevator_minimum, true },
		{ "elevator.maximum", _elevator_maximum, true },
		{ "ailerons.minimum", _ailerons_minimum, true },
		{ "ailerons.maximum", _ailerons_maximum, true },
		{ "rudder.minimum", _rudder_minimum, true },
		{ "rudder.maximum", _rudder_maximum, true },
		{ "output.serviceable", _serviceable, true },
		{ "output.control-stick-pitch", _output_control_stick_pitch, false },
		{ "output.control-stick-roll", _output_control_stick_roll, false },
		{ "output.pitch", _output_pitch, true },
		{ "output.roll", _output_roll, true },
		{ "output.elevator", _output_elevator, true },
		{ "output.ailerons", _output_ailerons, true },
		{ "output.rudder", _output_rudder, true },
		{ "output.throttle", _output_throttle, true },
	});

	_elevator_pid.set_i_limit ({ -0.1f, +0.1f });
	_elevator_pid.set_winding (true);
	_ailerons_pid.set_i_limit ({ -0.1f, +0.1f });
	_ailerons_pid.set_winding (true);
	_rudder_pid.set_i_limit ({ -0.1f, +0.1f });

	for (auto pid: { &_manual_pitch_pid, &_manual_roll_pid })
	{
		pid->set_i_limit ({ -0.05f, +0.05f });
		pid->set_winding (true);
	}

	_fbw_computer.set_minimum_dt (5_ms);
	_fbw_computer.set_callback (std::bind (&FlyByWire::compute_fbw, this));
	_fbw_computer.add_depending_smoothers ({
		&_elevator_smoother,
		&_ailerons_smoother,
	});
	_fbw_computer.observe ({
		&_attitude_mode,
		&_throttle_mode,
		&_pitch_extent,
		&_roll_extent,
		&_input_pitch_axis,
		&_input_roll_axis,
		&_input_yaw_axis,
		&_input_pitch,
		&_input_roll,
		&_input_throttle_axis,
		&_input_throttle,
		&_measured_pitch,
		&_measured_roll,
		&_measured_slip_skid_g,
		&_elevator_minimum,
		&_elevator_maximum,
		&_ailerons_minimum,
		&_ailerons_maximum,
		&_rudder_minimum,
		&_rudder_maximum,
	});
}


void
FlyByWire::data_updated()
{
	_fbw_computer.data_updated (update_time());
}


void
FlyByWire::rescue()
{
	if (_serviceable.configured())
		_serviceable.write (false);
}


void
FlyByWire::compute_fbw()
{
	Time update_dt = _fbw_computer.update_dt();

	double computed_elevator = 0.0;
	double computed_ailerons = 0.0;
	double computed_rudder = 0.0;

	if (_attitude_mode.valid())
	{
		switch (static_cast<AttitudeMode> (*_attitude_mode))
		{
			case AttitudeMode::Manual:
				_elevator_smoother.invalidate();
				_ailerons_smoother.invalidate();

				_computed_output_pitch = 0_deg;
				_computed_output_roll = 0_deg;

				computed_elevator = _input_pitch_axis.read (0.0);
				computed_ailerons = _input_roll_axis.read (0.0);
				computed_rudder = _input_yaw_axis.read (0.0);
				break;

			case AttitudeMode::Stabilized:
			case AttitudeMode::FlightDirector:
			{
				using Xefis::Range;

				if (_measured_pitch.is_nil() || _measured_roll.is_nil())
				{
					diagnose();

					_computed_output_pitch = 0_deg;
					_computed_output_roll = 0_deg;

					_serviceable.write (false);
				}
				else
				{
					// Should be computed for both Stabilized and FD modes:
					integrate_manual_input (update_dt);

					if (static_cast<AttitudeMode> (*_attitude_mode) == AttitudeMode::FlightDirector)
					{
						_computed_output_pitch = _input_pitch.read (0_deg);
						_computed_output_roll = _input_roll.read (0_deg);
					}

					_elevator_pid.set_pid (_pitch_p, _pitch_i, _pitch_d);
					_elevator_pid.set_gain (_pitch_gain * _stabilization_gain);
					_elevator_pid.set_error_power (_pitch_error_power);
					_elevator_pid.set_output_limit (Range<double> (_elevator_minimum.read (-1.0), _elevator_maximum.read (1.0)));
					_elevator_pid.set_target (_computed_output_pitch / 180_deg);
					_elevator_pid.process (*_measured_pitch / 180_deg, update_dt);

					_ailerons_pid.set_pid (_roll_p, _roll_i, _roll_d);
					_ailerons_pid.set_gain (_roll_gain * _stabilization_gain);
					_ailerons_pid.set_error_power (_roll_error_power);
					_ailerons_pid.set_output_limit (Range<double> (_ailerons_minimum.read (-1.0), _ailerons_maximum.read (1.0)));
					_ailerons_pid.set_target (_computed_output_roll / 180_deg);
					_ailerons_pid.process (*_measured_roll / 180_deg, update_dt);

					_rudder_pid.set_pid (_yaw_p, _yaw_i, _yaw_d);
					_rudder_pid.set_gain (_yaw_gain * _stabilization_gain);
					_rudder_pid.set_error_power (_yaw_error_power);
					_rudder_pid.set_output_limit (Range<double> (_rudder_minimum.read (-1.0), _rudder_maximum.read (1.0)));
					_rudder_pid.set_target (0.0);
					_rudder_pid.process (_measured_slip_skid_g.read (0.0), update_dt);

					computed_elevator = -std::cos (*_measured_roll) * _elevator_pid.output();
					computed_elevator = _elevator_smoother.process (computed_elevator, update_dt);

					computed_ailerons = _ailerons_pid.output();
					computed_ailerons = _ailerons_smoother.process (computed_ailerons, update_dt);

					// Mix manual rudder with auto-coordinated:
					double yaw_axis = _input_yaw_axis.read (0.0);
					_output_rudder.write (yaw_axis + (1.0f - yaw_axis) * _rudder_pid.output());

					_serviceable.write (true);
				}
				break;
			}

			default:
				log() << "unknown attitude mode: " << *_attitude_mode << std::endl;
				_serviceable.write (false);
				break;
		}
	}
	else
	{
		_serviceable.write (false);
	}

	if (_throttle_mode.valid())
	{
		switch (static_cast<ThrottleMode> (*_throttle_mode))
		{
			case ThrottleMode::Manual:
				_output_throttle.write (_input_throttle_axis.read (0.0));
				break;

			case ThrottleMode::Autothrottle:
				_output_throttle.write (_input_throttle.read (0.0));
				break;

			default:
				log() << "unknownn throttle mode: " << *_throttle_mode << std::endl;
				break;
		}
	}
	else
	{
		_serviceable.write (false);
	}

	// Output:
	if (_output_pitch.configured())
		_output_pitch.write (_computed_output_pitch);
	if (_output_roll.configured())
		_output_roll.write (_computed_output_roll);

	if (_output_elevator.configured())
		_output_elevator.write (computed_elevator);
	if (_output_ailerons.configured())
		_output_ailerons.write (computed_ailerons);
	if (_output_rudder.configured())
		_output_rudder.write (computed_rudder);
}


void
FlyByWire::integrate_manual_input (Time update_dt)
{
	using Xefis::floored_mod;

	if (_pitch_extent.is_nil())
		log() << "pitch-extent is nil, using default value" << std::endl;
	if (_roll_extent.is_nil())
		log() << "roll-extent is nil, using default value" << std::endl;

	Angle target_pitch_extent = _pitch_extent.read (5_deg);
	Angle target_roll_extent = _roll_extent.read (30_deg);

	if (_input_pitch_axis.is_nil())
		log() << "input.pitch-axis is nil, using 0.0" << std::endl;
	if (_input_roll_axis.is_nil())
		log() << "input.roll-axis is nil, using 0.0" << std::endl;

	double axis_pitch = _input_pitch_axis.read (0.0);
	double axis_roll = _input_roll_axis.read (0.0);

	Angle measured_pitch = *_measured_pitch;
	Angle measured_roll = *_measured_roll;

	// Target attitude - computed from current orientation and joystick deflection:
	Angle target_pitch = measured_pitch + std::cos (measured_roll) * axis_pitch * target_pitch_extent;
	Angle target_roll = measured_roll + axis_roll * target_roll_extent;
	target_pitch = floored_mod (target_pitch, -180_deg, +180_deg);
	target_roll = floored_mod (target_roll, -180_deg, +180_deg);

	// Update output pitch attitude:
	_manual_pitch_pid.set_target (target_pitch.deg() / 180.f);
	_manual_pitch_pid.process (_computed_output_pitch.deg() / 180.f, update_dt);
	_computed_output_pitch += std::abs (axis_pitch) * _manual_pitch_pid.output() * 360_deg;
	_computed_output_pitch = floored_mod (_computed_output_pitch, -180_deg, +180_deg);

	// Update output roll attitude:
	_manual_roll_pid.set_target (target_roll.deg() / 180.f);
	_manual_roll_pid.process (_computed_output_roll.deg() / 180.f, update_dt);
	_computed_output_roll += std::abs (axis_roll) * _manual_roll_pid.output() * 360_deg;
	_computed_output_roll = floored_mod (_computed_output_roll, -180_deg, +180_deg);

	// Joystick visualisation on EFIS:
	if (_output_control_stick_pitch.configured())
		_output_control_stick_pitch.write (axis_pitch * target_pitch_extent);
	if (_output_control_stick_roll.configured())
		_output_control_stick_roll.write (axis_roll * target_roll_extent);
}


void
FlyByWire::diagnose()
{
	if (_attitude_mode.is_nil())
		log() << "Attitude mode is nil!" << std::endl;
	if (_measured_pitch.is_nil())
		log() << "Measured pitch is nil!" << std::endl;
	if (_measured_roll.is_nil())
		log() << "Measured roll is nil!" << std::endl;
}

