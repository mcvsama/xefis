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

	void
	compute_speeds();

	Optional<Speed>
	get_stall_ias (Angle const& max_bank_angle) const;

	Optional<Speed>
	tas_to_ias (Speed const& tas) const;

	void
	compute_critical_aoa();

	void
	compute_C_L();

	void
	compute_C_D();

	void
	compute_estimations();

  private:
	Time						_total_energy_variometer_time		= 50_ms;
	Speed						_total_energy_variometer_min_ias	= 0_kt;
	double						_prev_total_energy					= 0.0;
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	Xefis::Smoother<double>		_wind_direction_smoother			= 5_s;
	Xefis::Smoother<double>		_wind_speed_smoother				= 5_s;
	Xefis::Smoother<double>		_total_energy_variometer_smoother	= 1_s;
	xf::Smoother<double>		_cl_smoother						= 1_s;
	// Input:
	Xefis::PropertySpeed		_speed_ias;
	Xefis::PropertySpeed		_speed_tas;
	Xefis::PropertySpeed		_speed_gs;
	Xefis::PropertySpeed		_vertical_speed;
	Xefis::PropertyLength		_altitude_amsl_std;
	Xefis::PropertyAngle		_track_lateral_true;
	Xefis::PropertyAngle		_orientation_heading_true;
	Xefis::PropertyAngle		_magnetic_declination;
	xf::PropertyLength			_density_altitude;
	xf::PropertyDensity			_input_air_density_static;
	xf::PropertyWeight			_input_aircraft_weight;
	Xefis::PropertyAngle		_input_flaps_angle;
	Xefis::PropertyAngle		_input_spoilers_angle;
	Xefis::PropertyAngle		_input_aoa_alpha;
	xf::PropertyAcceleration	_input_load;
	xf::PropertyAngle			_input_bank_angle;
	// Output:
	Xefis::PropertyAngle		_wind_from_true;
	Xefis::PropertyAngle		_wind_from_magnetic;
	Xefis::PropertySpeed		_wind_tas;
	Xefis::PropertyFloat		_glide_ratio;
	Xefis::PropertyString		_glide_ratio_string;
	Xefis::PropertySpeed		_total_energy_variometer;			// TODO change to Energy
	xf::PropertySpeed			_v_s;								// Current stall speed (depends on current bank angle)
	xf::PropertySpeed			_v_s_0_deg;							// Stall speed with wings level
	xf::PropertySpeed			_v_s_5_deg;							// Stall speed at 5° bank
	xf::PropertySpeed			_v_s_30_deg;						// Stall speed at 30° bank
	xf::PropertySpeed			_v_r;								// Rotation speed
	xf::PropertySpeed			_v_a;								// Max maneuvering speed
	xf::PropertySpeed			_v_approach;						// Approach speed
	xf::PropertyAngle			_critical_aoa;
	xf::PropertyBoolean			_stall;
	xf::PropertyFloat			_lift_coefficient;
	xf::PropertySpeed			_estimated_ias;
	xf::PropertySpeed			_estimated_ias_error;
	xf::PropertyAngle			_estimated_aoa;
	xf::PropertyAngle			_estimated_aoa_error;
	// Other:
	Xefis::PropertyObserver		_wind_computer;
	Xefis::PropertyObserver		_glide_ratio_computer;
	Xefis::PropertyObserver		_total_energy_variometer_computer;
	xf::PropertyObserver		_speeds_computer;
	xf::PropertyObserver		_aoa_computer;
	xf::PropertyObserver		_cl_computer;
	xf::PropertyObserver		_estimations_computer;
};

#endif
