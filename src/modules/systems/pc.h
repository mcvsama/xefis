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


class PerformanceComputer: public xf::Module
{
  public:
	// Ctor
	PerformanceComputer (xf::ModuleManager*, QDomElement const& config);

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

	void
	compute_speeds_vbg();

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

	/**
	 * Convert AOA to IAS for current environment and configuration.
	 * Automatically includes flaps/spoilers angle, so parameter @aoa
	 * should only be wings AOA.
	 *
	 * May return empty result if it's not possible to compute TAS.
	 */
	Optional<Speed>
	aoa_to_tas_now (Angle const& aoa, Optional<Acceleration> const& load = {}) const;

  private:
	Speed						_total_energy_variometer_min_ias	= 0_kt;
	Energy						_prev_total_energy					= 0_J;
	xf::Airframe*				_airframe;
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	xf::Smoother<double>		_wind_direction_smoother			= 5_s;
	xf::Smoother<double>		_wind_speed_smoother				= 5_s;
	xf::Smoother<double>		_total_energy_variometer_smoother	= 1_s;
	xf::Smoother<double>		_cl_smoother						= 1_s;
	// Input:
	xf::PropertySpeed			_speed_ias;
	xf::PropertySpeed			_speed_tas;
	xf::PropertySpeed			_speed_gs;
	xf::PropertySpeed			_vertical_speed;
	xf::PropertyLength			_altitude_amsl_std;
	xf::PropertyAngle			_track_lateral_true;
	xf::PropertyAngle			_orientation_heading_true;
	xf::PropertyAngle			_magnetic_declination;
	xf::PropertyLength			_density_altitude;
	xf::PropertyDensity			_input_air_density_static;
	xf::PropertyMass			_input_aircraft_mass;
	xf::PropertyAngle			_input_flaps_angle;
	xf::PropertyAngle			_input_spoilers_angle;
	xf::PropertyAngle			_input_aoa_alpha;
	xf::PropertyAcceleration	_input_load;
	xf::PropertyAngle			_input_bank_angle;
	// Output:
	xf::PropertyAngle			_wind_from_true;
	xf::PropertyAngle			_wind_from_magnetic;
	xf::PropertySpeed			_wind_tas;
	xf::PropertyFloat			_glide_ratio;
	xf::PropertyString			_glide_ratio_string;
	xf::PropertyPower			_total_energy_variometer;
	xf::PropertySpeed			_v_s;								// Current stall speed (depends on current bank angle)
	xf::PropertySpeed			_v_s_0_deg;							// Stall speed with wings level
	xf::PropertySpeed			_v_s_5_deg;							// Stall speed at 5° bank
	xf::PropertySpeed			_v_s_30_deg;						// Stall speed at 30° bank
	xf::PropertySpeed			_v_r;								// Rotation speed
	xf::PropertySpeed			_v_a;								// Max maneuvering speed
	xf::PropertySpeed			_v_approach;						// Approach speed
	xf::PropertySpeed			_v_1;								// One engine inoperative decision speed.
	xf::PropertySpeed			_v_bg;								// Best glide speed (maximum unpowered range)
	xf::PropertySpeed			_v_br;								// Best powered range speed
	xf::PropertySpeed			_v_md;								// Minimum descent speed (maximum time airborne unpowered)
	xf::PropertySpeed			_v_be;								// Best endurance speed (maximum time airborne powered)
	xf::PropertySpeed			_v_x;								// Best angle of climb (shortest ground distance climb)
	xf::PropertySpeed			_v_y;								// Best rate of climb (shortest time climb)
	xf::PropertyAngle			_critical_aoa;
	xf::PropertyBoolean			_stall;
	xf::PropertyFloat			_lift_coefficient;
	xf::PropertySpeed			_estimated_ias;
	xf::PropertySpeed			_estimated_ias_error;
	xf::PropertyAngle			_estimated_aoa;
	xf::PropertyAngle			_estimated_aoa_error;
	// Other:
	xf::PropertyObserver		_wind_computer;
	xf::PropertyObserver		_glide_ratio_computer;
	xf::PropertyObserver		_total_energy_variometer_computer;
	xf::PropertyObserver		_speeds_computer;
	xf::PropertyObserver		_aoa_computer;
	xf::PropertyObserver		_cl_computer;
	xf::PropertyObserver		_estimations_computer;
};

#endif
