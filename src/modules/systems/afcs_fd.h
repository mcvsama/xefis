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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_FD_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_FD_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/pid_control.h>
#include <xefis/utility/smoother.h>


class AFCS_FD: public Xefis::Module
{
  public:
	enum class RollMode
	{
		None			= 0,
		Heading			= 1,
		Track			= 2,
		sentinel		= 3,
	};

	enum class PitchMode
	{
		None			= 0,
		Altitude		= 1,
		Airspeed		= 2,
		VerticalSpeed	= 3,
		FPA				= 4,
		sentinel		= 5,
	};

  public:
	// Ctor
	AFCS_FD (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

	void
	rescue() override;

  private:
	/**
	 * Compute all needed data and write to output properties.
	 */
	void
	compute_fd();

	void
	roll_mode_changed();

	void
	pitch_mode_changed();

  private:
	double						_magnetic_heading_pid_p		= 1.0;
	double						_magnetic_heading_pid_i		= 0.1;
	double						_magnetic_heading_pid_d		= 0.0;
	double						_magnetic_track_pid_p		= 1.0;
	double						_magnetic_track_pid_i		= 0.1;
	double						_magnetic_track_pid_d		= 0.0;
	double						_altitude_pid_p				= 1.0;
	double						_altitude_pid_i				= 0.1;
	double						_altitude_pid_d				= 0.0;
	double						_ias_pid_p					= 1.0;
	double						_ias_pid_i					= 0.1;
	double						_ias_pid_d					= 0.0;
	double						_vertical_speed_pid_p		= 1.0;
	double						_vertical_speed_pid_i		= 0.1;
	double						_vertical_speed_pid_d		= 0.0;
	double						_fpa_pid_p					= 1.0;
	double						_fpa_pid_i					= 0.1;
	double						_fpa_pid_d					= 0.0;
	Xefis::PIDControl<double>	_magnetic_heading_pid;
	Xefis::PIDControl<double>	_magnetic_track_pid;
	Xefis::PIDControl<double>	_altitude_pid;
	Xefis::PIDControl<double>	_ias_pid;
	Xefis::PIDControl<double>	_vertical_speed_pid;
	Xefis::PIDControl<double>	_fpa_pid;
	Xefis::Smoother<double>		_output_pitch_smoother		= 2.5_s;
	Xefis::Smoother<double>		_output_roll_smoother		= 2.5_s;
	Angle						_computed_output_pitch;
	Angle						_computed_output_roll;
	RollMode					_roll_mode					= RollMode::None;
	PitchMode					_pitch_mode					= PitchMode::None;
	// Input:
	Xefis::PropertyAngle		_pitch_limit_max;
	Xefis::PropertyAngle		_pitch_limit_min;
	Xefis::PropertyAngle		_roll_limit;
	Xefis::PropertyInteger		_cmd_roll_mode;
	Xefis::PropertyInteger		_cmd_pitch_mode;
	Xefis::PropertyAngle		_cmd_magnetic_heading;
	Xefis::PropertyAngle		_cmd_magnetic_track;
	Xefis::PropertyLength		_cmd_altitude;
	Xefis::PropertySpeed		_cmd_ias;
	Xefis::PropertySpeed		_cmd_vertical_speed;
	Xefis::PropertyAngle		_cmd_fpa;
	Xefis::PropertyAngle		_measured_magnetic_heading;
	Xefis::PropertyAngle		_measured_magnetic_track;
	Xefis::PropertyLength		_measured_altitude;
	Xefis::PropertySpeed		_measured_ias;
	Xefis::PropertySpeed		_measured_vertical_speed;
	Xefis::PropertyAngle		_measured_fpa;
	// Output:
	Xefis::PropertyAngle		_output_pitch;
	Xefis::PropertyAngle		_output_roll;
	Xefis::PropertyBoolean		_operative;
	// Other:
	Xefis::PropertyObserver		_fd_computer;
};

#endif
