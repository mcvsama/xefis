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
#include <xefis/core/property.h>
#include <xefis/core/instrument.h>
#include <xefis/utility/smoother.h>

// Local:
#include "hsi_widget.h"


class HSI: public Xefis::Instrument
{
	Q_OBJECT

  public:
	// Ctor
	HSI (Xefis::ModuleManager*, QDomElement const& config);

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

	Xefis::PropertyInteger	_display_mode;
	Xefis::PropertyLength	_range;
	Xefis::PropertyLength	_trend_vector_range;
	Xefis::PropertySpeed	_speed_gs;
	Xefis::PropertySpeed	_speed_tas;
	Xefis::PropertyBoolean	_cmd_visible;
	Xefis::PropertyAngle	_cmd_heading;
	Xefis::PropertyBoolean	_cmd_track_visible;
	Xefis::PropertyLength	_target_altitude_reach_distance;
	Xefis::PropertyAngle	_orientation_heading_magnetic;
	Xefis::PropertyAngle	_orientation_heading_true;
	Xefis::PropertyBoolean	_use_true_heading;
	Xefis::PropertyAngle	_home_true_direction;
	Xefis::PropertyBoolean	_home_track_visible;
	Xefis::PropertyLength	_home_distance_vlos;
	Xefis::PropertyLength	_home_distance_ground;
	Xefis::PropertyLength	_home_distance_vertical;
	Xefis::PropertyAngle	_home_position_longitude;
	Xefis::PropertyAngle	_home_position_latitude;
	Xefis::PropertyAngle	_position_latitude;
	Xefis::PropertyAngle	_position_longitude;
	Xefis::PropertyString	_position_source;
	Xefis::PropertyBoolean	_track_visible;
	Xefis::PropertyAngle	_track_lateral_magnetic;
	Xefis::PropertyAngle	_track_lateral_delta_dpm;
	Xefis::PropertyBoolean	_track_center_on_track;
	Xefis::PropertyBoolean	_course_visible;
	Xefis::PropertyAngle	_course_setting_magnetic;
	Xefis::PropertyAngle	_course_deviation;
	Xefis::PropertyBoolean	_course_to_flag;
	Xefis::PropertyString	_navaid_selected_reference;
	Xefis::PropertyString	_navaid_selected_identifier;
	Xefis::PropertyLength	_navaid_selected_distance;
	Xefis::PropertyTime		_navaid_selected_eta;
	Xefis::PropertyAngle	_navaid_selected_course_magnetic;
	Xefis::PropertyString	_navaid_left_reference;
	Xefis::PropertyString	_navaid_left_identifier;
	Xefis::PropertyLength	_navaid_left_distance;
	Xefis::PropertyAngle	_navaid_left_reciprocal_magnetic;
	Xefis::PropertyString	_navaid_right_reference;
	Xefis::PropertyString	_navaid_right_identifier;
	Xefis::PropertyLength	_navaid_right_distance;
	Xefis::PropertyAngle	_navaid_right_reciprocal_magnetic;
	Xefis::PropertyAngle	_wind_from_magnetic;
	Xefis::PropertySpeed	_wind_speed_tas;
	Xefis::PropertyString	_localizer_id;
	Xefis::PropertyBoolean	_tcas_on;
	Xefis::PropertyLength	_tcas_range;
};


inline void
HSI::data_updated()
{
	read();
}

#endif
