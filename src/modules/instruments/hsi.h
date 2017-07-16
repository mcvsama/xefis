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
#include <xefis/core/instrument.h>
#include <xefis/utility/smoother.h>

// Local:
#include "hsi_widget.h"


class HSI: public xf::Instrument
{
	Q_OBJECT

  public:
	// Ctor
	HSI (xf::ModuleManager*, QDomElement const& config);

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
	xf::PropertyInteger		_display_mode;
	xf::PropertyLength		_range;
	xf::PropertySpeed		_speed_gs;
	xf::PropertySpeed		_speed_tas;
	xf::PropertyBoolean		_cmd_visible;
	xf::PropertyBoolean		_cmd_line_visible;
	xf::PropertyAngle		_cmd_heading_magnetic;
	xf::PropertyAngle		_cmd_track_magnetic;
	xf::PropertyBoolean		_cmd_use_trk;
	xf::PropertyLength		_target_altitude_reach_distance;
	xf::PropertyAngle		_orientation_heading_magnetic;
	xf::PropertyAngle		_orientation_heading_true;
	xf::PropertyBoolean		_use_true_heading;
	xf::PropertyAngle		_home_true_direction;
	xf::PropertyBoolean		_home_track_visible;
	xf::PropertyLength		_home_distance_vlos;
	xf::PropertyLength		_home_distance_ground;
	xf::PropertyLength		_home_distance_vertical;
	xf::PropertyAngle		_home_position_longitude;
	xf::PropertyAngle		_home_position_latitude;
	xf::PropertyAngle		_position_latitude;
	xf::PropertyAngle		_position_longitude;
	xf::PropertyString		_position_source;
	xf::PropertyBoolean		_track_visible;
	xf::PropertyAngle		_track_lateral_magnetic;
	xf::Property<AngularVelocity>
							_track_lateral_rotation;
	xf::PropertyBoolean		_track_center_on_track;
	xf::PropertyBoolean		_course_visible;
	xf::PropertyAngle		_course_setting_magnetic;
	xf::PropertyAngle		_course_deviation;
	xf::PropertyBoolean		_course_to_flag;
	xf::PropertyString		_navaid_selected_reference;
	xf::PropertyString		_navaid_selected_identifier;
	xf::PropertyLength		_navaid_selected_distance;
	xf::PropertyTime		_navaid_selected_eta;
	xf::PropertyAngle		_navaid_selected_course_magnetic;
	xf::PropertyInteger		_navaid_left_type;
	xf::PropertyString		_navaid_left_reference;
	xf::PropertyString		_navaid_left_identifier;
	xf::PropertyLength		_navaid_left_distance;
	xf::PropertyAngle		_navaid_left_initial_bearing_magnetic;
	xf::PropertyInteger		_navaid_right_type;
	xf::PropertyString		_navaid_right_reference;
	xf::PropertyString		_navaid_right_identifier;
	xf::PropertyLength		_navaid_right_distance;
	xf::PropertyAngle		_navaid_right_initial_bearing_magnetic;
	xf::PropertyLength		_navigation_required_performance;
	xf::PropertyLength		_navigation_actual_performance;
	xf::PropertyAngle		_wind_from_magnetic;
	xf::PropertySpeed		_wind_speed_tas;
	xf::PropertyString		_localizer_id;
	xf::PropertyBoolean		_tcas_on;
	xf::PropertyLength		_tcas_range;
	xf::PropertyBoolean		_features_fix;
	xf::PropertyBoolean		_features_vor;
	xf::PropertyBoolean		_features_dme;
	xf::PropertyBoolean		_features_ndb;
	xf::PropertyBoolean		_features_loc;
	xf::PropertyBoolean		_features_arpt;
};


inline void
HSI::data_updated()
{
	read();
}

#endif
