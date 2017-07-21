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

#ifndef XEFIS__MODULES__INSTRUMENTS__HSI_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__HSI_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/property.h>
#include <xefis/core/v1/instrument.h>
#include <xefis/utility/smoother.h>

// Local:
#include "hsi_widget.h"


class HSI: public v1::Instrument
{
	Q_OBJECT

  public:
	// Ctor
	HSI (v1::ModuleManager*, QDomElement const& config);

  public slots:
	/**
	 * Force HSI to read data from properties.
	 */
	void
	read();

  protected:
	void
	data_updated() override;

  private:
	Unique<HSIWidget>		_hsi_widget;
	std::array<LonLat, 3>	_positions;
	bool					_positions_valid = false;
	// Settings:
	Length					_arpt_runways_range_threshold;
	Length					_arpt_map_range_threshold;
	Length					_arpt_runway_extension_length;
	std::array<Time, 3>		_trend_vector_times				= { { 30_s, 60_s, 90_s } };
	std::array<Length, 3>	_trend_vector_min_ranges		= { { 5_nmi, 10_nmi, 15_nmi } };
	Length					_trend_vector_max_range			= 30_nmi;
	// Input:
	v1::PropertyInteger		_display_mode;
	v1::PropertyLength		_range;
	v1::PropertySpeed		_speed_gs;
	v1::PropertySpeed		_speed_tas;
	v1::PropertyBoolean		_cmd_visible;
	v1::PropertyBoolean		_cmd_line_visible;
	v1::PropertyAngle		_cmd_heading_magnetic;
	v1::PropertyAngle		_cmd_track_magnetic;
	v1::PropertyBoolean		_cmd_use_trk;
	v1::PropertyLength		_target_altitude_reach_distance;
	v1::PropertyAngle		_orientation_heading_magnetic;
	v1::PropertyAngle		_orientation_heading_true;
	v1::PropertyBoolean		_use_true_heading;
	v1::PropertyAngle		_home_true_direction;
	v1::PropertyBoolean		_home_track_visible;
	v1::PropertyLength		_home_distance_vlos;
	v1::PropertyLength		_home_distance_ground;
	v1::PropertyLength		_home_distance_vertical;
	v1::PropertyAngle		_home_position_longitude;
	v1::PropertyAngle		_home_position_latitude;
	v1::PropertyAngle		_position_latitude;
	v1::PropertyAngle		_position_longitude;
	v1::PropertyString		_position_source;
	v1::PropertyBoolean		_track_visible;
	v1::PropertyAngle		_track_lateral_magnetic;
	v1::Property<AngularVelocity>
							_track_lateral_rotation;
	v1::PropertyBoolean		_track_center_on_track;
	v1::PropertyBoolean		_course_visible;
	v1::PropertyAngle		_course_setting_magnetic;
	v1::PropertyAngle		_course_deviation;
	v1::PropertyBoolean		_course_to_flag;
	v1::PropertyString		_navaid_selected_reference;
	v1::PropertyString		_navaid_selected_identifier;
	v1::PropertyLength		_navaid_selected_distance;
	v1::PropertyTime		_navaid_selected_eta;
	v1::PropertyAngle		_navaid_selected_course_magnetic;
	v1::PropertyInteger		_navaid_left_type;
	v1::PropertyString		_navaid_left_reference;
	v1::PropertyString		_navaid_left_identifier;
	v1::PropertyLength		_navaid_left_distance;
	v1::PropertyAngle		_navaid_left_initial_bearing_magnetic;
	v1::PropertyInteger		_navaid_right_type;
	v1::PropertyString		_navaid_right_reference;
	v1::PropertyString		_navaid_right_identifier;
	v1::PropertyLength		_navaid_right_distance;
	v1::PropertyAngle		_navaid_right_initial_bearing_magnetic;
	v1::PropertyLength		_navigation_required_performance;
	v1::PropertyLength		_navigation_actual_performance;
	v1::PropertyAngle		_wind_from_magnetic;
	v1::PropertySpeed		_wind_speed_tas;
	v1::PropertyString		_localizer_id;
	v1::PropertyBoolean		_tcas_on;
	v1::PropertyLength		_tcas_range;
	v1::PropertyBoolean		_features_fix;
	v1::PropertyBoolean		_features_vor;
	v1::PropertyBoolean		_features_dme;
	v1::PropertyBoolean		_features_ndb;
	v1::PropertyBoolean		_features_loc;
	v1::PropertyBoolean		_features_arpt;
};


inline void
HSI::data_updated()
{
	read();
}

#endif
