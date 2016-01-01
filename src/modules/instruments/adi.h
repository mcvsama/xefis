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

#ifndef XEFIS__MODULES__INSTRUMENTS__ADI_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__ADI_H__INCLUDED

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
#include "adi_widget.h"


class ADI: public xf::Instrument
{
	Q_OBJECT

  public:
	// Ctor
	ADI (xf::ModuleManager*, QDomElement const& config);

  public slots:
	/**
	 * Force ADI to read data from properties.
	 */
	void
	read();

  protected:
	void
	data_updated() override;

	void
	compute_fpv();

  private:
	ADIWidget*						_adi_widget						= nullptr;
	// Setting
	xf::PropertyInteger::Type		_speed_ladder_line_every		= 10;
	xf::PropertyInteger::Type		_speed_ladder_number_every		= 20;
	xf::PropertyInteger::Type		_speed_ladder_extent			= 124;
	xf::PropertyInteger::Type		_speed_ladder_minimum			= 20;
	xf::PropertyInteger::Type		_speed_ladder_maximum			= 350;
	xf::PropertyInteger::Type		_altitude_ladder_line_every		= 100;
	xf::PropertyInteger::Type		_altitude_ladder_number_every	= 200;
	xf::PropertyInteger::Type		_altitude_ladder_emphasis_every	= 1000;
	xf::PropertyInteger::Type		_altitude_ladder_bold_every		= 500;
	xf::PropertyInteger::Type		_altitude_ladder_extent			= 825;
	Length							_altitude_landing_warning_hi	= 1000_ft;
	Length							_altitude_landing_warning_lo	= 500_ft;
	Length							_raising_runway_visibility		= 1000_ft;
	Length							_raising_runway_threshold		= 250_ft;
	Angle							_aoa_visibility_threshold		= 17.5_deg;
	double							_show_mach_above				= 0.4;
	Power							_1000_fpm_power					= 1000_W;
	// Speed
	xf::PropertyBoolean				_speed_ias_serviceable;
	xf::PropertySpeed				_speed_ias;
	xf::PropertySpeed				_speed_ias_lookahead;
	xf::PropertySpeed				_speed_ias_minimum;
	xf::PropertySpeed				_speed_ias_minimum_maneuver;
	xf::PropertySpeed				_speed_ias_maximum_maneuver;
	xf::PropertySpeed				_speed_ias_maximum;
	xf::PropertyFloat				_speed_mach;
	xf::PropertySpeed				_speed_ground;
	// Speed bugs
	xf::PropertySpeed				_speed_v1;
	xf::PropertySpeed				_speed_vr;
	xf::PropertySpeed				_speed_vref;
	xf::PropertyString				_speed_flaps_up_label;
	xf::PropertySpeed				_speed_flaps_up_speed;
	QString							_speed_flaps_up_current_label;
	xf::PropertyString				_speed_flaps_a_label;
	xf::PropertySpeed				_speed_flaps_a_speed;
	QString							_speed_flaps_a_current_label;
	xf::PropertyString				_speed_flaps_b_label;
	xf::PropertySpeed				_speed_flaps_b_speed;
	QString							_speed_flaps_b_current_label;
	// Attitude and heading
	xf::PropertyBoolean				_orientation_serviceable;
	xf::PropertyAngle				_orientation_pitch;
	xf::PropertyAngle				_orientation_roll;
	xf::PropertyAngle				_orientation_heading_magnetic;
	xf::PropertyAngle				_orientation_heading_true;
	xf::PropertyBoolean				_orientation_heading_numbers_visible;
	// Track
	xf::PropertyAngle				_track_lateral_magnetic;
	xf::PropertyAngle				_track_lateral_true;
	xf::PropertyAngle				_track_vertical;
	// Slip-skid indicator
	xf::PropertyFloat				_slip_skid;
	// Flight Path Vector
	xf::PropertyBoolean				_fpv_visible;
	xf::PropertyBoolean				_weight_on_wheels;
	bool							_computed_fpv_failure			= false;
	bool							_computed_fpv_visible			= false;
	Angle							_computed_fpv_alpha				= 0_deg;
	Angle							_computed_fpv_beta				= 0_deg;
	// Angle of Attack
	xf::PropertyAngle				_aoa_alpha;
	xf::PropertyAngle				_aoa_alpha_maximum;
	xf::PropertyBoolean				_aoa_alpha_visible;
	// Pressure and radio altitude
	xf::PropertyBoolean				_altitude_amsl_serviceable;
	xf::PropertyLength				_altitude_amsl;
	xf::PropertyLength				_altitude_amsl_lookahead;
	xf::PropertyBoolean				_altitude_agl_serviceable;
	xf::PropertyLength				_altitude_agl;
	xf::PropertyString				_altitude_minimums_type;
	xf::PropertyLength				_altitude_minimums_setting;
	xf::PropertyLength				_altitude_minimums_amsl;
	xf::PropertyLength				_altitude_landing_amsl;
	// Vertical speed
	xf::PropertyBoolean				_vertical_speed_serviceable;
	xf::PropertySpeed				_vertical_speed;
	xf::PropertyPower				_vertical_speed_energy_variometer;
	// Air pressure settings
	xf::PropertyPressure			_pressure_qnh;
	xf::PropertyBoolean				_pressure_display_hpa;
	xf::PropertyBoolean				_pressure_use_std;
	// Flight director
	xf::PropertyBoolean				_flight_director_serviceable;
	xf::PropertyBoolean				_flight_director_cmd_visible;
	xf::PropertyLength				_flight_director_cmd_altitude;
	xf::PropertyBoolean				_flight_director_cmd_altitude_acquired;
	xf::PropertySpeed				_flight_director_cmd_ias;
	xf::PropertyFloat				_flight_director_cmd_mach;
	xf::PropertySpeed				_flight_director_cmd_vertical_speed;
	xf::PropertyAngle				_flight_director_cmd_fpa;
	xf::PropertyBoolean				_flight_director_guidance_visible;
	xf::PropertyAngle				_flight_director_guidance_pitch;
	xf::PropertyAngle				_flight_director_guidance_roll;
	// Stick position indicator
	xf::PropertyBoolean				_control_stick_visible;
	xf::PropertyAngle				_control_stick_pitch;
	xf::PropertyAngle				_control_stick_roll;
	// Approach information
	xf::PropertyBoolean				_navaid_reference_visible;
	xf::PropertyAngle				_navaid_course_magnetic;
	xf::PropertyString				_navaid_type_hint;
	xf::PropertyString				_navaid_identifier;
	xf::PropertyLength				_navaid_distance;
	// Flight path deviation
	xf::PropertyBoolean				_flight_path_deviation_lateral_serviceable;
	xf::PropertyAngle				_flight_path_deviation_lateral_app;
	xf::PropertyAngle				_flight_path_deviation_lateral_fp;
	xf::PropertyBoolean				_flight_path_deviation_vertical_serviceable;
	xf::PropertyAngle				_flight_path_deviation_vertical;
	xf::PropertyAngle				_flight_path_deviation_vertical_app;
	xf::PropertyAngle				_flight_path_deviation_vertical_fp;
	xf::PropertyBoolean				_flight_path_deviation_mixed_mode;
	// Flight mode information
	xf::PropertyBoolean				_flight_mode_hint_visible;
	xf::PropertyString				_flight_mode_hint;
	xf::PropertyBoolean				_flight_mode_fma_visible;
	xf::PropertyString				_flight_mode_fma_speed_hint;
	xf::PropertyString				_flight_mode_fma_speed_small_hint;
	xf::PropertyString				_flight_mode_fma_lateral_hint;
	xf::PropertyString				_flight_mode_fma_lateral_small_hint;
	xf::PropertyString				_flight_mode_fma_vertical_hint;
	xf::PropertyString				_flight_mode_fma_vertical_small_hint;
	// TCAS
	xf::PropertyAngle				_tcas_resolution_advisory_pitch_minimum;
	xf::PropertyAngle				_tcas_resolution_advisory_pitch_maximum;
	xf::PropertySpeed				_tcas_resolution_advisory_vertical_speed_minimum;
	xf::PropertySpeed				_tcas_resolution_advisory_vertical_speed_maximum;
	// General warning/failure flags
	xf::PropertyBoolean				_warning_novspd_flag;
	xf::PropertyBoolean				_warning_ldgalt_flag;
	xf::PropertyBoolean				_warning_pitch_disagree;
	xf::PropertyBoolean				_warning_roll_disagree;
	xf::PropertyBoolean				_warning_ias_disagree;
	xf::PropertyBoolean				_warning_altitude_disagree;
	xf::PropertyBoolean				_warning_roll;
	xf::PropertyBoolean				_warning_slip_skid;
	// Style
	xf::PropertyBoolean				_style_old;
	xf::PropertyBoolean				_style_show_metric;
	// Other:
	xf::PropertyObserver			_fpv_computer;
};


inline void
ADI::data_updated()
{
	read();
}

#endif
