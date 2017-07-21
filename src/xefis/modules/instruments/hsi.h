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
#include <xefis/core/v2/instrument.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/core/xefis.h>
#include <xefis/support/navigation/navaid_storage.h>

// Local:
#include "hsi_widget.h"


class HSI: public v2::Instrument
{
	Q_OBJECT

  public:
	/*
	 * Settings
	 */

	// At what ranve setting to start drawing airport circles:
	v2::Setting<si::Length>					arpt_runways_range_threshold			{ this };
	// At what range setting to start drawing runways instead of circles:
	v2::Setting<si::Length>					arpt_map_range_threshold				{ this };
	// Length of the runway extension line on the map:
	v2::Setting<si::Length>					arpt_runway_extension_length			{ this };
	v2::Setting<std::array<si::Time, 3>>	trend_vector_times						{ this, { 30_s, 60_s, 90_s } };
	v2::Setting<std::array<si::Length, 3>>	trend_vector_min_ranges					{ this, { 5_nmi, 10_nmi, 15_nmi } };
	v2::Setting<si::Length>					trend_vector_max_range					{ this, 30_nmi };

	/*
	 * Input
	 */

	v2::PropertyIn<int64_t>					display_mode							{ this, "/display-mode", 0 }; // TODO use enum
	v2::PropertyIn<si::Length>				range									{ this, "/range", 5_nmi };
	v2::PropertyIn<si::Velocity>			speed_gs								{ this, "/speeds/gs" };
	v2::PropertyIn<si::Velocity>			speed_tas								{ this, "/speeds/tas" };
	v2::PropertyIn<bool>					cmd_visible								{ this, "/cmd/visible" };
	v2::PropertyIn<bool>					cmd_line_visible						{ this, "/cmd/line-visible" };
	v2::PropertyIn<si::Angle>				cmd_heading_magnetic					{ this, "/cmd/heading-magnetic" };
	v2::PropertyIn<si::Angle>				cmd_track_magnetic						{ this, "/cmd/track-magnetic" };
	v2::PropertyIn<bool>					cmd_use_trk								{ this, "/cmd/use-trk" };
	v2::PropertyIn<si::Length>				target_altitude_reach_distance			{ this, "/target-altitude-reach-distance" };
	v2::PropertyIn<si::Angle>				orientation_heading_magnetic			{ this, "/orientation/heading-magnetic" };
	v2::PropertyIn<si::Angle>				orientation_heading_true				{ this, "/orientation/heading-true" };
	v2::PropertyIn<bool>					use_true_heading						{ this, "/use-true-heading" };
	v2::PropertyIn<si::Angle>				home_true_direction						{ this, "/home/true-direction" };
	v2::PropertyIn<bool>					home_track_visible						{ this, "/home/track-visible" };
	v2::PropertyIn<si::Length>				home_distance_vlos						{ this, "/home/distance/vlos" };
	v2::PropertyIn<si::Length>				home_distance_ground					{ this, "/home/distance/ground" };
	v2::PropertyIn<si::Length>				home_distance_vertical					{ this, "/home/distance/vertical" };
	v2::PropertyIn<si::Angle>				home_position_longitude					{ this, "/home/position/longitude" };
	v2::PropertyIn<si::Angle>				home_position_latitude					{ this, "/home/position/latitude" };
	v2::PropertyIn<si::Angle>				position_longitude						{ this, "/position/longitude" };
	v2::PropertyIn<si::Angle>				position_latitude						{ this, "/position/latitude" };
	v2::PropertyIn<std::string>				position_source							{ this, "/position/source" };
	v2::PropertyIn<bool>					track_visible							{ this, "/track/visible" };
	v2::PropertyIn<si::Angle>				track_lateral_magnetic					{ this, "/track/lateral-magnetic" };
	v2::PropertyIn<si::AngularVelocity>		track_lateral_rotation					{ this, "/track/lateral-rotation" };
	v2::PropertyIn<bool>					track_center_on_track					{ this, "/track/center-on-track" };
	v2::PropertyIn<bool>					course_visible							{ this, "/course/visible" };
	v2::PropertyIn<si::Angle>				course_setting_magnetic					{ this, "/course/setting-magnetic" };
	v2::PropertyIn<si::Angle>				course_deviation						{ this, "/course/deviation" };
	v2::PropertyIn<bool>					course_to_flag							{ this, "/course/to-flag" };
	v2::PropertyIn<std::string>				navaid_selected_reference				{ this, "/navaid/selected/reference" };
	v2::PropertyIn<std::string>				navaid_selected_identifier				{ this, "/navaid/selected/identifier" };
	v2::PropertyIn<si::Length>				navaid_selected_distance				{ this, "/navaid/selected/distance" };
	v2::PropertyIn<si::Time>				navaid_selected_eta						{ this, "/navaid/selected/eta" };
	v2::PropertyIn<si::Angle>				navaid_selected_course_magnetic			{ this, "/navaid/selected/course-magnetic" };
	v2::PropertyIn<int64_t>					navaid_left_type						{ this, "/navaid/left/type" };
	v2::PropertyIn<std::string>				navaid_left_reference					{ this, "/navaid/left/reference" };
	v2::PropertyIn<std::string>				navaid_left_identifier					{ this, "/navaid/left/identifier" };
	v2::PropertyIn<si::Length>				navaid_left_distance					{ this, "/navaid/left/distance" };
	v2::PropertyIn<si::Angle>				navaid_left_initial_bearing_magnetic	{ this, "/navaid/left/initial-bearing-magnetic" };
	v2::PropertyIn<int64_t>					navaid_right_type						{ this, "/navaid/right/type" };
	v2::PropertyIn<std::string>				navaid_right_reference					{ this, "/navaid/right/reference" };
	v2::PropertyIn<std::string>				navaid_right_identifier					{ this, "/navaid/right/identifier" };
	v2::PropertyIn<si::Length>				navaid_right_distance					{ this, "/navaid/right/distance" };
	v2::PropertyIn<si::Angle>				navaid_right_initial_bearing_magnetic	{ this, "/navaid/right/initial-bearing-magnetic" };
	v2::PropertyIn<si::Length>				navigation_required_performance			{ this, "/navigation/required-performance" };
	v2::PropertyIn<si::Length>				navigation_actual_performance			{ this, "/navigation/actual-performance" };
	v2::PropertyIn<si::Angle>				wind_from_magnetic						{ this, "/wind/from-magnetic" };
	v2::PropertyIn<si::Velocity>			wind_speed_tas							{ this, "/wind/speed-tas" };
	v2::PropertyIn<std::string>				localizer_id							{ this, "/localizer-id" };
	v2::PropertyIn<bool>					tcas_on									{ this, "/tcas/on" };
	v2::PropertyIn<si::Length>				tcas_range								{ this, "/tcas/range" };
	v2::PropertyIn<bool>					features_fix							{ this, "/features/fix" };
	v2::PropertyIn<bool>					features_vor							{ this, "/features/vor" };
	v2::PropertyIn<bool>					features_dme							{ this, "/features/dme" };
	v2::PropertyIn<bool>					features_ndb							{ this, "/features/ndb" };
	v2::PropertyIn<bool>					features_loc							{ this, "/features/loc" };
	v2::PropertyIn<bool>					features_arpt							{ this, "/features/arpt" };

  public:
	// Ctor
	HSI (xf::Xefis*, xf::NavaidStorage*, std::string const& instance = {});

	// Module API
	void
	process (v2::Cycle const&) override;

  private:
	Unique<HSIWidget>	_hsi_widget;
};

#endif
