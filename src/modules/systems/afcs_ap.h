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


/**
 * Steers control surfaces (ailerons, elevator) to obtain desired orientation
 * (pitch, roll).
 */
class AFCS_AP: public xf::Module
{
  public:
	// Ctor
	AFCS_AP (xf::ModuleManager*, QDomElement const& config);

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
	xf::PIDControl<double>	_elevator_pid;
	xf::PIDControl<double>	_ailerons_pid;
	xf::Smoother<double>	_elevator_smoother		= Time (50_ms);
	xf::Smoother<double>	_ailerons_smoother		= Time (50_ms);
	// Settings:
	double					_stabilization_gain;
	double					_pitch_gain;
	double					_pitch_p;
	double					_pitch_i;
	double					_pitch_d;
	double					_pitch_error_power;
	double					_roll_gain;
	double					_roll_p;
	double					_roll_i;
	double					_roll_d;
	double					_roll_error_power;
	double					_yaw_gain;
	double					_yaw_p;
	double					_yaw_i;
	double					_yaw_d;
	double					_yaw_error_power;
	// Input:
	xf::PropertyAngle		_input_pitch;
	xf::PropertyAngle		_input_roll;
	xf::PropertyAngle		_measured_pitch;
	xf::PropertyAngle		_measured_roll;
	xf::PropertyFloat		_elevator_minimum;
	xf::PropertyFloat		_elevator_maximum;
	xf::PropertyFloat		_ailerons_minimum;
	xf::PropertyFloat		_ailerons_maximum;
	// Output:
	xf::PropertyBoolean		_serviceable;
	xf::PropertyFloat		_output_elevator;
	xf::PropertyFloat		_output_ailerons;
	// Other:
	xf::PropertyObserver	_ap_computer;
};

#endif
