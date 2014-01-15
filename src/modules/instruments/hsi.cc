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


XEFIS_REGISTER_MODULE_CLASS ("instruments/hsi", HSI);


HSI::HSI (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Instrument (module_manager, config)
{
	parse_properties (config, {
		{ "display-mode", _display_mode, true },
		{ "range", _range, true },
		{ "trend-vector-range", _trend_vector_range, false },
		{ "speed.gs", _speed_gs, false },
		{ "speed.tas", _speed_tas, false },
		{ "cmd.visible", _cmd_visible, false },
		{ "cmd.heading", _cmd_heading, false },
		{ "cmd.track-visible", _cmd_track_visible, false },
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
		{ "track.lateral.delta", _track_lateral_delta_dpm, false },
		{ "track.center-on-track", _track_center_on_track, false },
		{ "course.visible", _course_visible, false },
		{ "course.setting.magnetic", _course_setting_magnetic, false },
		{ "course.deviation", _course_deviation, false },
		{ "course.to-flag", _course_to_flag, false },
		{ "navaid.selected.visible", _navaid_visible, false },
		{ "navaid.selected.reference", _navaid_reference, false },
		{ "navaid.selected.identifier", _navaid_identifier, false },
		{ "navaid.selected.distance", _navaid_distance, false },
		{ "navaid.selected.eta", _navaid_eta, false },
		{ "navaid.selected.course.magnetic", _navaid_course_magnetic, false },
		{ "navaid.left.visible", _navaid_left_visible, false },
		{ "navaid.left.reference", _navaid_left_reference, false },
		{ "navaid.left.identifier", _navaid_left_identifier, false },
		{ "navaid.left.distance", _navaid_left_distance, false },
		{ "navaid.left.reciprocal.magnetic", _navaid_left_reciprocal_magnetic, false },
		{ "navaid.right.visible", _navaid_right_visible, false },
		{ "navaid.right.reference", _navaid_right_reference, false },
		{ "navaid.right.identifier", _navaid_right_identifier, false },
		{ "navaid.right.distance", _navaid_right_distance, false },
		{ "navaid.right.reciprocal.magnetic", _navaid_right_reciprocal_magnetic, false },
		{ "wind.from.magnetic", _wind_from_magnetic, false },
		{ "wind.tas", _wind_speed_tas, false },
		{ "localizer-id", _localizer_id, false },
		{ "tcas.on", _tcas_on, false },
		{ "tcas.range", _tcas_range, false },
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
	params.range = _range.read (5_nm);
	params.heading_visible = _orientation_heading_magnetic.valid();
	params.heading_magnetic = *_orientation_heading_magnetic;
	params.heading_true = *_orientation_heading_true;
	params.ap_heading_visible = _cmd_visible.read (false) && _cmd_heading.valid();
	params.ap_track_visible = _cmd_track_visible.read (false);
	params.ap_magnetic_heading = *_cmd_heading;
	params.track_visible = _track_visible.read (false) && (_track_lateral_magnetic.valid() || _orientation_heading_magnetic.valid());
	params.track_magnetic = _track_lateral_magnetic.read (*_orientation_heading_magnetic);
	params.course_visible = _course_visible.read (false);
	params.course_setting_magnetic = _course_setting_magnetic.get_optional();
	params.course_deviation = _course_deviation.get_optional();
	params.course_to_flag = _course_to_flag.get_optional();
	params.navaid_visible = _navaid_visible.read (false);
	params.navaid_identifier = QString::fromStdString (_navaid_identifier.read (""));
	params.navaid_reference = QString::fromStdString (_navaid_reference.read (""));
	params.navaid_distance = _navaid_distance.get_optional();
	params.navaid_eta = _navaid_eta.get_optional();
	params.navaid_course_magnetic = _navaid_course_magnetic.get_optional();
	params.navaid_left_visible = _navaid_left_visible.read (false);
	params.navaid_left_reference = QString::fromStdString (_navaid_left_reference.read (""));
	params.navaid_left_identifier = QString::fromStdString (_navaid_left_identifier.read (""));
	params.navaid_left_distance = _navaid_left_distance.get_optional();
	params.navaid_left_reciprocal_magnetic = _navaid_left_reciprocal_magnetic.get_optional();
	params.navaid_right_visible = _navaid_right_visible.read (false);
	params.navaid_right_reference = QString::fromStdString (_navaid_right_reference.read (""));
	params.navaid_right_identifier = QString::fromStdString (_navaid_right_identifier.read (""));
	params.navaid_right_distance = _navaid_right_distance.get_optional();
	params.navaid_right_reciprocal_magnetic = _navaid_right_reciprocal_magnetic.get_optional();
	params.center_on_track = _track_center_on_track.read (true);
	params.home_direction_visible = _home_true_direction.valid();
	params.home_track_visible = _home_track_visible.read (false);
	params.true_home_direction = *_home_true_direction;
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
	params.ground_speed_visible = _speed_gs.valid();
	params.ground_speed = *_speed_gs;
	params.true_air_speed_visible = _speed_tas.valid();
	params.true_air_speed = *_speed_tas;
	params.trend_vector_visible = _track_lateral_delta_dpm.valid();
	params.track_lateral_delta = Xefis::limit (*_track_lateral_delta_dpm, -180.0_deg, +180.0_deg);
	params.trend_vector_lookahead = *_trend_vector_range;
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
	params.vor_visible = true;
	params.dme_visible = true;
	params.ndb_visible = true;
	params.loc_visible = true;
	params.fix_visible = true;
	params.highlighted_loc = QString::fromStdString (_localizer_id.read (""));
	params.positioning_hint_visible = _position_source.valid();
	params.positioning_hint = QString::fromStdString (*_position_source);
	params.tcas_on = _tcas_on.get_optional();
	params.tcas_range = _tcas_range.get_optional();
	params.round_clip = false;

	_hsi_widget->set_params (params);
}

