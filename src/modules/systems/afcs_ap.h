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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_AP_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_AP_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/pid_control.h>
#include <xefis/utility/smoother.h>


class AFCS_AP: public Xefis::Module
{
  public:
	// Ctor
	AFCS_AP (Xefis::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

	void
	rescue() override;

	/**
	 * Do all FBW computations and write to output properties.
	 */
	void
	compute_ap();

	/**
	 * Check properties and diagnose problem on the log.
	 */
	void
	diagnose();

  private:
	// Stabilizer PIDs:
	Xefis::PIDControl<double>	_elevator_pid;
	Xefis::PIDControl<double>	_ailerons_pid;
	Xefis::Smoother<double>		_elevator_smoother		= 50_ms;
	Xefis::Smoother<double>		_ailerons_smoother		= 50_ms;
	// Settings:
	double						_stabilization_gain;
	double						_pitch_gain;
	double						_pitch_p;
	double						_pitch_i;
	double						_pitch_d;
	double						_pitch_error_power;
	double						_roll_gain;
	double						_roll_p;
	double						_roll_i;
	double						_roll_d;
	double						_roll_error_power;
	double						_yaw_gain;
	double						_yaw_p;
	double						_yaw_i;
	double						_yaw_d;
	double						_yaw_error_power;
	// Input:
	Xefis::PropertyAngle		_input_pitch;
	Xefis::PropertyAngle		_input_roll;
	Xefis::PropertyAngle		_measured_pitch;
	Xefis::PropertyAngle		_measured_roll;
	Xefis::PropertyFloat		_elevator_minimum;
	Xefis::PropertyFloat		_elevator_maximum;
	Xefis::PropertyFloat		_ailerons_minimum;
	Xefis::PropertyFloat		_ailerons_maximum;
	// Output:
	Xefis::PropertyBoolean		_serviceable;
	Xefis::PropertyFloat		_output_elevator;
	Xefis::PropertyFloat		_output_ailerons;
	// Other:
	Xefis::PropertyObserver		_ap_computer;
};

#endif
