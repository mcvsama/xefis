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

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/services.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "hsi.h"


HSI::HSI (xf::Xefis* xefis, xf::NavaidStorage* navaid_storage, std::string const& instance):
	Instrument (instance)
{
	_hsi_widget = std::make_unique<HSIWidget> (this, xefis->work_performer());
	_hsi_widget->set_navaid_storage (navaid_storage);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_hsi_widget.get());
}


void
HSI::process (v2::Cycle const&)
{
	HSIWidget::Parameters params;

	switch (display_mode.value_or (0))
	{
		case 0:		params.display_mode = HSIWidget::DisplayMode::Expanded; break;
		case 1:		params.display_mode = HSIWidget::DisplayMode::Rose; break;
		default:	params.display_mode = HSIWidget::DisplayMode::Auxiliary; break;
	}

	params.heading_mode = use_true_heading.value_or (false) ? HSIWidget::HeadingMode::True : HSIWidget::HeadingMode::Magnetic;
	params.range = range.value_or (5_nmi);
	params.heading_magnetic = orientation_heading_magnetic.get_optional();
	params.heading_true = orientation_heading_true.get_optional();
	params.ap_visible = cmd_visible.value_or (false);
	params.ap_line_visible = cmd_line_visible.value_or (false);
	params.ap_heading_magnetic = cmd_heading_magnetic.get_optional();
	params.ap_track_magnetic = cmd_track_magnetic.get_optional();
	params.ap_use_trk = cmd_use_trk.get_optional();
	params.track_visible = track_visible.value_or (false) && track_lateral_magnetic;
	params.track_magnetic = track_lateral_magnetic.get_optional();
	params.course_visible = course_visible.value_or (false);
	params.course_setting_magnetic = course_setting_magnetic.get_optional();
	params.course_deviation = course_deviation.get_optional();
	params.course_to_flag = course_to_flag.get_optional();
	params.navaid_selected_reference = QString::fromStdString (navaid_selected_reference.value_or (""));
	params.navaid_selected_identifier = QString::fromStdString (navaid_selected_identifier.value_or (""));
	params.navaid_selected_distance = navaid_selected_distance.get_optional();
	params.navaid_selected_eta = navaid_selected_eta.get_optional();
	params.navaid_selected_course_magnetic = navaid_selected_course_magnetic.get_optional();
	params.navaid_left_reference = QString::fromStdString (navaid_left_reference.value_or (""));
	params.navaid_left_type = navaid_left_type.value_or (0);
	params.navaid_left_identifier = QString::fromStdString (navaid_left_identifier.value_or (""));
	params.navaid_left_distance = navaid_left_distance.get_optional();
	params.navaid_left_initial_bearing_magnetic = navaid_left_initial_bearing_magnetic.get_optional();
	params.navaid_right_type = navaid_right_type.value_or (0);
	params.navaid_right_reference = QString::fromStdString (navaid_right_reference.value_or (""));
	params.navaid_right_identifier = QString::fromStdString (navaid_right_identifier.value_or (""));
	params.navaid_right_distance = navaid_right_distance.get_optional();
	params.navaid_right_initial_bearing_magnetic = navaid_right_initial_bearing_magnetic.get_optional();
	params.navigation_required_performance = navigation_required_performance.get_optional();
	params.navigation_actual_performance = navigation_actual_performance.get_optional();
	params.center_on_track = track_center_on_track.value_or (true);
	params.home_track_visible = home_track_visible.value_or (false);
	params.true_home_direction = home_true_direction.get_optional();
	params.dist_to_home_ground = home_distance_ground.get_optional();
	params.dist_to_home_vlos = home_distance_vlos.get_optional();
	params.dist_to_home_vert = home_distance_vertical.get_optional();

	if (home_position_longitude && home_position_latitude)
		params.home = LonLat (*home_position_longitude, *home_position_latitude);
	else
		params.home.reset();

	params.ground_speed = speed_gs.get_optional();
	params.true_air_speed = speed_tas.get_optional();
	params.track_lateral_rotation = track_lateral_rotation.get_optional();

	if (params.track_lateral_rotation)
		params.track_lateral_rotation = xf::clamped<si::AngularVelocity> (*params.track_lateral_rotation, -1_Hz, +1_Hz);

	params.altitude_reach_distance = target_altitude_reach_distance.get_optional();
	params.wind_from_magnetic_heading = wind_from_magnetic.get_optional();
	params.wind_tas_speed = wind_speed_tas.get_optional();

	if (position_longitude && position_latitude)
		params.position = LonLat (*position_longitude, *position_latitude);
	else
		params.position.reset();

	params.navaids_visible = orientation_heading_true.valid();
	params.fix_visible = features_fix.value_or (false);
	params.vor_visible = features_vor.value_or (false);
	params.dme_visible = features_dme.value_or (false);
	params.ndb_visible = features_ndb.value_or (false);
	params.loc_visible = features_loc.value_or (false);
	params.arpt_visible = features_arpt.value_or (false);
	params.highlighted_loc = QString::fromStdString (localizer_id.value_or (""));

	if (position_source)
		params.positioning_hint = QString::fromStdString (*position_source);
	else
		params.positioning_hint.reset();

	params.tcas_on = tcas_on.get_optional();
	params.tcas_range = tcas_range.get_optional();
	params.arpt_runways_range_threshold = *arpt_runways_range_threshold;
	params.arpt_map_range_threshold = *arpt_map_range_threshold;
	params.arpt_runway_extension_length = *arpt_runway_extension_length;
	params.trend_vector_times = *trend_vector_times;
	params.trend_vector_min_ranges = *trend_vector_min_ranges;
	params.trend_vector_max_range = *trend_vector_max_range;
	params.round_clip = false;

	_hsi_widget->set_params (params);
}

