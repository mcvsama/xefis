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


XEFIS_REGISTER_MODULE_CLASS ("instruments/hsi", HSI)


HSI::HSI (v1::ModuleManager* module_manager, QDomElement const& config):
	Instrument (module_manager, config)
{
	parse_settings (config, {
		{ "arpt.runways-range-threshold", _arpt_runways_range_threshold, false },
		{ "arpt.map-range-threshold", _arpt_map_range_threshold, false },
		{ "arpt.runway-extension-length", _arpt_runway_extension_length, false },
		{ "trend-vector.vertex.0.time", _trend_vector_times[0], false },
		{ "trend-vector.vertex.1.time", _trend_vector_times[1], false },
		{ "trend-vector.vertex.2.time", _trend_vector_times[2], false },
		{ "trend-vector.vertex.0.minimum-range", _trend_vector_min_ranges[0], false },
		{ "trend-vector.vertex.1.minimum-range", _trend_vector_min_ranges[1], false },
		{ "trend-vector.vertex.2.minimum-range", _trend_vector_min_ranges[2], false },
		{ "trend-vector.maximum-range", _trend_vector_max_range, false },
	});

	parse_properties (config, {
		{ "display-mode", _display_mode, true },
		{ "range", _range, true },
		{ "speed.gs", _speed_gs, false },
		{ "speed.tas", _speed_tas, false },
		{ "cmd.visible", _cmd_visible, false },
		{ "cmd.line-visible", _cmd_line_visible, false },
		{ "cmd.heading.magnetic", _cmd_heading_magnetic, false },
		{ "cmd.track.magnetic", _cmd_track_magnetic, false },
		{ "cmd.use-trk", _cmd_use_trk, false },
		{ "altitude.target.reach-distance", _target_altitude_reach_distance, false },
		{ "orientation.heading.magnetic", _orientation_heading_magnetic, false },
		{ "orientation.heading.true", _orientation_heading_true, false },
		{ "orientation.display-true-heading", _use_true_heading, false },
		{ "home.true-direction", _home_true_direction, false },
		{ "home.track-visible", _home_track_visible, false },
		{ "home.distance.vlos", _home_distance_vlos, false },
		{ "home.distance.ground", _home_distance_ground, false },
		{ "home.distance.vertical", _home_distance_vertical, false },
		{ "home.position.longitude", _home_position_longitude, false },
		{ "home.position.latitude", _home_position_latitude, false },
		{ "position.latitude", _position_latitude, false },
		{ "position.longitude", _position_longitude, false },
		{ "position.source", _position_source, false },
		{ "track.visible", _track_visible, false },
		{ "track.lateral.magnetic", _track_lateral_magnetic, false },
		{ "track.lateral.rotation", _track_lateral_rotation, false },
		{ "track.center-on-track", _track_center_on_track, false },
		{ "course.visible", _course_visible, false },
		{ "course.setting.magnetic", _course_setting_magnetic, false },
		{ "course.deviation", _course_deviation, false },
		{ "course.to-flag", _course_to_flag, false },
		{ "navaid.selected.reference", _navaid_selected_reference, false },
		{ "navaid.selected.identifier", _navaid_selected_identifier, false },
		{ "navaid.selected.distance", _navaid_selected_distance, false },
		{ "navaid.selected.eta", _navaid_selected_eta, false },
		{ "navaid.selected.course.magnetic", _navaid_selected_course_magnetic, false },
		{ "navaid.left.type", _navaid_left_type, false },
		{ "navaid.left.reference", _navaid_left_reference, false },
		{ "navaid.left.identifier", _navaid_left_identifier, false },
		{ "navaid.left.distance", _navaid_left_distance, false },
		{ "navaid.left.initial-bearing.magnetic", _navaid_left_initial_bearing_magnetic, false },
		{ "navaid.right.type", _navaid_right_type, false },
		{ "navaid.right.reference", _navaid_right_reference, false },
		{ "navaid.right.identifier", _navaid_right_identifier, false },
		{ "navaid.right.distance", _navaid_right_distance, false },
		{ "navaid.right.initial-bearing.magnetic", _navaid_right_initial_bearing_magnetic, false },
		{ "navigation.required-performance", _navigation_required_performance, false },
		{ "navigation.actual-performance", _navigation_actual_performance, false },
		{ "wind.from.magnetic", _wind_from_magnetic, false },
		{ "wind.tas", _wind_speed_tas, false },
		{ "localizer-id", _localizer_id, false },
		{ "tcas.on", _tcas_on, false },
		{ "tcas.range", _tcas_range, false },
		{ "features.fix", _features_fix, false },
		{ "features.vor", _features_vor, false },
		{ "features.dme", _features_dme, false },
		{ "features.ndb", _features_ndb, false },
		{ "features.loc", _features_loc, false },
		{ "features.arpt", _features_arpt, false },
	});

	_hsi_widget = std::make_unique<HSIWidget> (this, work_performer());
	_hsi_widget->set_navaid_storage (navaid_storage());

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_hsi_widget.get());
}


void
HSI::read()
{
	HSIWidget::Parameters params;

	switch (_display_mode.read (0))
	{
		case 0:		params.display_mode = HSIWidget::DisplayMode::Expanded; break;
		case 1:		params.display_mode = HSIWidget::DisplayMode::Rose; break;
		default:	params.display_mode = HSIWidget::DisplayMode::Auxiliary; break;
	}
	params.heading_mode = _use_true_heading.read (false) ? HSIWidget::HeadingMode::True : HSIWidget::HeadingMode::Magnetic;
	params.range = _range.read (5_nmi);
	params.heading_visible = _orientation_heading_magnetic.valid();
	params.heading_magnetic = *_orientation_heading_magnetic;
	params.heading_true = *_orientation_heading_true;
	params.ap_visible = _cmd_visible.read (false);
	params.ap_line_visible = _cmd_line_visible.read (false);
	params.ap_heading_magnetic = _cmd_heading_magnetic.get_optional();
	params.ap_track_magnetic = _cmd_track_magnetic.get_optional();
	params.ap_use_trk = _cmd_use_trk.get_optional();
	params.track_visible = _track_visible.read (false) && (_track_lateral_magnetic.valid() || _orientation_heading_magnetic.valid());
	params.track_magnetic = _track_lateral_magnetic.read (*_orientation_heading_magnetic);
	params.course_visible = _course_visible.read (false);
	params.course_setting_magnetic = _course_setting_magnetic.get_optional();
	params.course_deviation = _course_deviation.get_optional();
	params.course_to_flag = _course_to_flag.get_optional();
	params.navaid_selected_reference = QString::fromStdString (_navaid_selected_reference.read (""));
	params.navaid_selected_identifier = QString::fromStdString (_navaid_selected_identifier.read (""));
	params.navaid_selected_distance = _navaid_selected_distance.get_optional();
	params.navaid_selected_eta = _navaid_selected_eta.get_optional();
	params.navaid_selected_course_magnetic = _navaid_selected_course_magnetic.get_optional();
	params.navaid_left_reference = QString::fromStdString (_navaid_left_reference.read (""));
	params.navaid_left_type = _navaid_left_type.read (0);
	params.navaid_left_identifier = QString::fromStdString (_navaid_left_identifier.read (""));
	params.navaid_left_distance = _navaid_left_distance.get_optional();
	params.navaid_left_initial_bearing_magnetic = _navaid_left_initial_bearing_magnetic.get_optional();
	params.navaid_right_type = _navaid_right_type.read (0);
	params.navaid_right_reference = QString::fromStdString (_navaid_right_reference.read (""));
	params.navaid_right_identifier = QString::fromStdString (_navaid_right_identifier.read (""));
	params.navaid_right_distance = _navaid_right_distance.get_optional();
	params.navaid_right_initial_bearing_magnetic = _navaid_right_initial_bearing_magnetic.get_optional();
	params.navigation_required_performance = _navigation_required_performance.get_optional();
	params.navigation_actual_performance = _navigation_actual_performance.get_optional();
	params.center_on_track = _track_center_on_track.read (true);
	params.home_track_visible = _home_track_visible.read (false);
	params.true_home_direction = _home_true_direction.get_optional();
	params.dist_to_home_ground_visible = _home_distance_ground.valid();
	params.dist_to_home_ground = *_home_distance_ground;
	params.dist_to_home_vlos_visible = _home_distance_vlos.valid();
	params.dist_to_home_vlos = *_home_distance_vlos;
	params.dist_to_home_vert_visible = _home_distance_vertical.valid();
	params.dist_to_home_vert = *_home_distance_vertical;
	if (_home_position_longitude.valid() && _home_position_latitude.valid())
		params.home = LonLat (*_home_position_longitude, *_home_position_latitude);
	else
		params.home.reset();
	params.ground_speed = _speed_gs.get_optional();
	params.true_air_speed = _speed_tas.get_optional();
	params.track_lateral_rotation = _track_lateral_rotation.get_optional();
	if (params.track_lateral_rotation)
		params.track_lateral_rotation = xf::clamped<si::AngularVelocity> (*params.track_lateral_rotation, -1_Hz, +1_Hz);
	params.altitude_reach_visible = _target_altitude_reach_distance.valid();
	params.altitude_reach_distance = *_target_altitude_reach_distance;
	params.wind_information_visible = _wind_from_magnetic.valid() && _wind_speed_tas.valid();
	params.wind_from_magnetic_heading = *_wind_from_magnetic;
	params.wind_tas_speed = *_wind_speed_tas;
	params.position_valid = _position_latitude.valid() && _position_longitude.valid();
	if (params.position_valid)
		params.position = LonLat (*_position_longitude, *_position_latitude);
	else
		params.position.reset();
	params.navaids_visible = _orientation_heading_true.valid();
	params.fix_visible = _features_fix.read (false);
	params.vor_visible = _features_vor.read (false);
	params.dme_visible = _features_dme.read (false);
	params.ndb_visible = _features_ndb.read (false);
	params.loc_visible = _features_loc.read (false);
	params.arpt_visible = _features_arpt.read (false);
	params.highlighted_loc = QString::fromStdString (_localizer_id.read (""));
	params.positioning_hint_visible = _position_source.valid();
	params.positioning_hint = QString::fromStdString (*_position_source);
	params.tcas_on = _tcas_on.get_optional();
	params.tcas_range = _tcas_range.get_optional();
	params.arpt_runways_range_threshold = _arpt_runways_range_threshold;
	params.arpt_map_range_threshold = _arpt_map_range_threshold;
	params.arpt_runway_extension_length = _arpt_runway_extension_length;
	params.trend_vector_times = _trend_vector_times;
	params.trend_vector_min_ranges = _trend_vector_min_ranges;
	params.trend_vector_max_range = _trend_vector_max_range;
	params.round_clip = false;

	_hsi_widget->set_params (params);
}

