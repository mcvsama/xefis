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

#ifndef XEFIS__MODULES__SYSTEMS__PC_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__PC_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/smoother.h>


class PerformanceComputer: public Xefis::Module
{
  public:
	// Ctor
	PerformanceComputer (Xefis::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

	void
	compute_wind();

	void
	compute_glide_ratio();

	void
	compute_total_energy_variometer();

  private:
	Time						_total_energy_variometer_time		= 100_ms;
	Speed						_total_energy_variometer_min_ias	= 0_kt;
	double						_prev_total_energy					= 0.0;
	Xefis::PropertyObserver		_wind_computer;
	Xefis::PropertyObserver		_glide_ratio_computer;
	Xefis::PropertyObserver		_total_energy_variometer_computer;
	Xefis::Smoother<double>		_wind_direction_smoother			= 10_s;
	Xefis::Smoother<double>		_total_energy_variometer_smoother	= 200_ms;
	// Input:
	Xefis::PropertyFloat		_aircraft_weight_g;
	Xefis::PropertySpeed		_speed_ias;
	Xefis::PropertySpeed		_speed_tas;
	Xefis::PropertySpeed		_speed_gs;
	Xefis::PropertySpeed		_vertical_speed;
	Xefis::PropertyLength		_altitude_amsl_std;
	Xefis::PropertyAngle		_track_lateral_true;
	Xefis::PropertyAngle		_orientation_heading_true;
	Xefis::PropertyAngle		_magnetic_declination;
	// Output:
	Xefis::PropertyAngle		_wind_from_true;
	Xefis::PropertyAngle		_wind_from_magnetic;
	Xefis::PropertySpeed		_wind_tas;
	Xefis::PropertyFloat		_glide_ratio;
	Xefis::PropertySpeed		_total_energy_variometer;
};

#endif
