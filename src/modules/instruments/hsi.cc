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
	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				{ "display-mode", _display_mode, true },
				{ "range", _range, true },
				{ "trend-vector-range", _trend_vector_range, false },
				{ "speed.gs", _gs, false },
				{ "speed.tas", _tas, false },
				{ "altitude", _altitude, false },
				{ "altitude.target", _target_altitude, false },
				{ "altitude.target.reach-distance", _target_altitude_reach_distance, false },
				{ "orientation.heading.magnetic", _magnetic_heading, false },
				{ "orientation.heading.true", _true_heading, false },
				{ "orientation.display-true-heading", _use_true_heading, false },
				{ "home.true-direction", _true_home_direction, false },
				{ "home.distance.vlos", _home_dist_vlos, false },
				{ "home.distance.ground", _home_dist_ground, false },
				{ "home.distance.vertical", _home_dist_vertical, false },
				{ "cmd.visible", _cmd_settings_visible, false },
				{ "cmd.heading", _cmd_heading_setting, false },
				{ "cmd.track-visible", _cmd_track_visible, false },
				{ "position.latitude", _position_lat, false },
				{ "position.longitude", _position_lon, false },
				{ "position.source", _positioning_hint, false },
				{ "track.visible", _display_track, false },
				{ "track.lateral-magnetic", _magnetic_track, false },
				{ "track.delta.lateral", _track_lateral_delta_dpm, false },
				{ "wind.from-mag-heading", _wind_from_magnetic_heading, false },
				{ "wind.tas", _wind_tas, false },
				{ "localizer-id", _localizer_id, false },
				{ "climb-glide-ratio", _climb_glide_ratio, false },
			});
		}
	}

	_hsi_widget = new HSIWidget (this, work_performer());
	_hsi_widget->set_navaid_storage (navaid_storage());

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_hsi_widget);
}


HSI::~HSI()
{
	delete _hsi_widget;
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
	params.heading_mode = _use_true_heading.read (false) ? HSIWidget::HeadingMode::Magnetic : HSIWidget::HeadingMode::True;
	params.range = _range.read (5_nm);
	params.heading_visible = _magnetic_heading.valid();
	params.heading_magnetic = *_magnetic_heading;
	params.heading_true = *_true_heading;
	params.ap_heading_visible = _cmd_settings_visible.read (false) && _cmd_heading_setting.valid();
	params.ap_track_visible = _cmd_track_visible.read (false);
	params.ap_magnetic_heading = *_cmd_heading_setting;
	params.track_visible = _magnetic_track.valid();
	params.track_magnetic = params.track_visible ? *_magnetic_track : *_magnetic_heading;
	params.display_track = _display_track.read (false);
	params.home_direction_visible = _true_home_direction.valid();
	params.true_home_direction = *_true_home_direction;
	params.dist_to_home_ground_visible = _home_dist_ground.valid();
	params.dist_to_home_ground = *_home_dist_ground;
	params.dist_to_home_vlos_visible = _home_dist_vlos.valid();
	params.dist_to_home_vlos = *_home_dist_vlos;
	params.dist_to_home_vert_visible = _home_dist_vertical.valid();
	params.dist_to_home_vert = *_home_dist_vertical;
	params.ground_speed_visible = _gs.valid();
	params.ground_speed = *_gs;
	params.true_air_speed_visible = _tas.valid();
	params.true_air_speed = *_tas;
	params.trend_vector_visible = _track_lateral_delta_dpm.valid();
	params.track_lateral_delta = Xefis::limit (*_track_lateral_delta_dpm, -180.0_deg, +180.0_deg);
	params.trend_vector_lookahead = *_trend_vector_range;
	params.altitude_reach_visible = _target_altitude_reach_distance.valid();
	params.altitude_reach_distance = *_target_altitude_reach_distance;
	params.wind_information_visible = _wind_from_magnetic_heading.valid() && _wind_tas.valid();
	params.wind_from_magnetic_heading = *_wind_from_magnetic_heading;
	params.wind_tas_speed = *_wind_tas;
	params.position_valid = _position_lat.valid() && _position_lon.valid();
	params.position = LonLat (*_position_lon, *_position_lat);
	params.navaids_visible = _true_heading.valid();
	params.vor_visible = true;
	params.dme_visible = true;
	params.ndb_visible = true;
	params.loc_visible = true;
	params.fix_visible = true;
	params.highlighted_loc = QString::fromStdString (_localizer_id.read (""));
	params.positioning_hint_visible = _positioning_hint.valid();
	params.positioning_hint = QString::fromStdString (*_positioning_hint);
	params.climb_glide_ratio_visible = _climb_glide_ratio.valid();
	params.climb_glide_ratio = *_climb_glide_ratio;
	params.round_clip = false;

	_hsi_widget->set_params (params);
}

