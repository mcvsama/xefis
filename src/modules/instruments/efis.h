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

#ifndef XEFIS__MODULES__INSTRUMENTS__EFIS_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__EFIS_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/core/instrument.h>

// Local:
#include "efis_widget.h"


class EFIS: public Xefis::Instrument
{
	Q_OBJECT

  public:
	// Ctor
	EFIS (Xefis::ModuleManager*, QDomElement const& config);

  public slots:
	/**
	 * Force EFIS to read data from properties.
	 */
	void
	read();

  protected:
	void
	data_updated() override;

	void
	compute_fpv();

  private:
	EFISWidget*						_efis_widget					= nullptr;
	Xefis::PropertyObserver			_fpv_computer;
	// Setting
	Xefis::PropertyInteger::Type	_speed_ladder_line_every		= 10;
	Xefis::PropertyInteger::Type	_speed_ladder_number_every		= 20;
	Xefis::PropertyInteger::Type	_speed_ladder_extent			= 124;
	Xefis::PropertyInteger::Type	_speed_ladder_minimum			= 20;
	Xefis::PropertyInteger::Type	_speed_ladder_maximum			= 350;
	Xefis::PropertyInteger::Type	_altitude_ladder_line_every		= 100;
	Xefis::PropertyInteger::Type	_altitude_ladder_number_every	= 200;
	Xefis::PropertyInteger::Type	_altitude_ladder_emphasis_every	= 1000;
	Xefis::PropertyInteger::Type	_altitude_ladder_bold_every		= 500;
	Xefis::PropertyInteger::Type	_altitude_ladder_extent			= 825;
	Length							_altitude_landing_warning_hi	= 1000_ft;
	Length							_altitude_landing_warning_lo	= 500_ft;
	Length							_raising_runway_visibility		= 1000_ft;
	Length							_raising_runway_threshold		= 250_ft;
	Angle							_aoa_visibility_threshold		= 17.5_deg;
	// Speed
	Xefis::PropertyBoolean			_speed_ias_serviceable;
	Xefis::PropertySpeed			_speed_ias;
	Xefis::PropertySpeed			_speed_ias_lookahead;
	Xefis::PropertySpeed			_speed_ias_minimum;
	Xefis::PropertySpeed			_speed_ias_minimum_maneuver;
	Xefis::PropertySpeed			_speed_ias_maximum_maneuver;
	Xefis::PropertySpeed			_speed_ias_maximum;
	Xefis::PropertyFloat			_speed_mach;
	// Speed bugs
	Xefis::PropertySpeed			_speed_v1;
	Xefis::PropertySpeed			_speed_vr;
	Xefis::PropertySpeed			_speed_vref;
	// Attitude and heading
	Xefis::PropertyBoolean			_orientation_serviceable;
	Xefis::PropertyAngle			_orientation_pitch;
	Xefis::PropertyAngle			_orientation_roll;
	Xefis::PropertyAngle			_orientation_heading_magnetic;
	Xefis::PropertyAngle			_orientation_heading_true;
	Xefis::PropertyBoolean			_orientation_heading_numbers_visible;
	// Track
	Xefis::PropertyAngle			_track_lateral_magnetic;
	Xefis::PropertyAngle			_track_lateral_true;
	Xefis::PropertyAngle			_track_vertical;
	// Slip-skid indicator
	Xefis::PropertyFloat			_slip_skid;
	// Flight Path Vector
	Xefis::PropertyBoolean			_fpv_visible;
	bool							_computed_fpv_failure			= false;
	bool							_computed_fpv_visible			= false;
	Angle							_computed_fpv_alpha				= 0_deg;
	Angle							_computed_fpv_beta				= 0_deg;
	// Angle of Attack
	Xefis::PropertyAngle			_aoa_alpha;
	Xefis::PropertyAngle			_aoa_alpha_maximum;
	Xefis::PropertyBoolean			_aoa_alpha_visible;
	// Pressure and radio altitude
	Xefis::PropertyBoolean			_altitude_amsl_serviceable;
	Xefis::PropertyLength			_altitude_amsl;
	Xefis::PropertyLength			_altitude_amsl_lookahead;
	Xefis::PropertyBoolean			_altitude_agl_serviceable;
	Xefis::PropertyLength			_altitude_agl;
	Xefis::PropertyString			_altitude_minimums_type;
	Xefis::PropertyLength			_altitude_minimums_setting;
	Xefis::PropertyLength			_altitude_minimums_amsl;
	Xefis::PropertyLength			_altitude_landing_amsl;
	// Vertical speed
	Xefis::PropertyBoolean			_vertical_speed_serviceable;
	Xefis::PropertySpeed			_vertical_speed;
	Xefis::PropertySpeed			_vertical_speed_variometer;
	// Air pressure settings
	Xefis::PropertyPressure			_pressure_qnh;
	Xefis::PropertyBoolean			_pressure_display_hpa;
	Xefis::PropertyBoolean			_pressure_use_std;
	// Flight director
	Xefis::PropertyBoolean			_flight_director_serviceable;
	Xefis::PropertyBoolean			_flight_director_cmd_visible;
	Xefis::PropertyLength			_flight_director_cmd_altitude;
	Xefis::PropertyBoolean			_flight_director_cmd_altitude_acquired;
	Xefis::PropertySpeed			_flight_director_cmd_ias;
	Xefis::PropertySpeed			_flight_director_cmd_vertical_speed;
	Xefis::PropertyBoolean			_flight_director_guidance_visible;
	Xefis::PropertyAngle			_flight_director_guidance_pitch;
	Xefis::PropertyAngle			_flight_director_guidance_roll;
	// Stick position indicator
	Xefis::PropertyBoolean			_control_stick_visible;
	Xefis::PropertyAngle			_control_stick_pitch;
	Xefis::PropertyAngle			_control_stick_roll;
	// Approach information
	Xefis::PropertyBoolean			_approach_reference_visible;
	Xefis::PropertyString			_approach_type_hint;
	Xefis::PropertyString			_approach_localizer_id;
	Xefis::PropertyLength			_approach_distance;
	// Flight path deviation
	Xefis::PropertyBoolean			_flight_path_deviation_lateral_serviceable;
	Xefis::PropertyAngle			_flight_path_deviation_lateral_app;
	Xefis::PropertyAngle			_flight_path_deviation_lateral_fp;
	Xefis::PropertyBoolean			_flight_path_deviation_vertical_serviceable;
	Xefis::PropertyAngle			_flight_path_deviation_vertical;
	Xefis::PropertyAngle			_flight_path_deviation_vertical_app;
	Xefis::PropertyAngle			_flight_path_deviation_vertical_fp;
	Xefis::PropertyBoolean			_flight_path_deviation_mixed_mode;
	// Flight mode information
	Xefis::PropertyBoolean			_flight_mode_hint_visible;
	Xefis::PropertyString			_flight_mode_hint;
	Xefis::PropertyBoolean			_flight_mode_fma_visible;
	Xefis::PropertyString			_flight_mode_fma_speed_hint;
	Xefis::PropertyString			_flight_mode_fma_speed_small_hint;
	Xefis::PropertyString			_flight_mode_fma_lateral_hint;
	Xefis::PropertyString			_flight_mode_fma_lateral_small_hint;
	Xefis::PropertyString			_flight_mode_fma_vertical_hint;
	Xefis::PropertyString			_flight_mode_fma_vertical_small_hint;
	// TCAS
	Xefis::PropertyAngle			_tcas_resolution_advisory_pitch_minimum;
	Xefis::PropertyAngle			_tcas_resolution_advisory_pitch_maximum;
	Xefis::PropertySpeed			_tcas_resolution_advisory_vertical_speed_minimum;
	Xefis::PropertySpeed			_tcas_resolution_advisory_vertical_speed_maximum;
	// General warning/failure flags
	Xefis::PropertyBoolean			_warning_novspd_flag;
	Xefis::PropertyBoolean			_warning_ldgalt_flag;
	Xefis::PropertyBoolean			_warning_pitch_disagree;
	Xefis::PropertyBoolean			_warning_roll_disagree;
	Xefis::PropertyBoolean			_warning_ias_disagree;
	Xefis::PropertyBoolean			_warning_altitude_disagree;
	Xefis::PropertyBoolean			_warning_roll;
	Xefis::PropertyBoolean			_warning_slip_skid;
	// Style
	Xefis::PropertyBoolean			_style_old;
	Xefis::PropertyBoolean			_style_show_metric;
};


inline void
EFIS::data_updated()
{
	read();
}

#endif
