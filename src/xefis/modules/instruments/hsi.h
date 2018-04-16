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
#include <xefis/core/instrument.h>
#include <xefis/core/property.h>
#include <xefis/core/setting.h>
#include <xefis/core/xefis.h>
#include <xefis/support/navigation/navaid_storage.h>
#include <xefis/support/system/work_performer.h>

// Local:
#include "hsi_widget.h"


class HSI_IO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	// At what ranve setting to start drawing airport circles:
	xf::Setting<si::Length>					arpt_runways_range_threshold			{ this, "arpt_runways_range_threshold" };
	// At what range setting to start drawing runways instead of circles:
	xf::Setting<si::Length>					arpt_map_range_threshold				{ this, "arpt_map_range_threshold" };
	// Length of the runway extension line on the map:
	xf::Setting<si::Length>					arpt_runway_extension_length			{ this, "arpt_runway_extension_length" };
	xf::Setting<std::array<si::Time, 3>>	trend_vector_durations					{ this, "trend_vector_durations", { 30_s, 60_s, 90_s } };
	xf::Setting<std::array<si::Length, 3>>	trend_vector_min_ranges					{ this, "trend_vector_min_ranges", { 5_nmi, 10_nmi, 15_nmi } };
	xf::Setting<si::Length>					trend_vector_max_range					{ this, "trend_vector_max_range", 30_nmi };

	/*
	 * Input
	 */

	xf::PropertyIn<int64_t>					display_mode							{ this, "/display-mode", 0 }; // TODO use enum
	xf::PropertyIn<si::Length>				range									{ this, "/range", 5_nmi };
	xf::PropertyIn<si::Velocity>			speed_gs								{ this, "/speeds/gs" };
	xf::PropertyIn<si::Velocity>			speed_tas								{ this, "/speeds/tas" };
	xf::PropertyIn<bool>					cmd_visible								{ this, "/cmd/visible" };
	xf::PropertyIn<bool>					cmd_line_visible						{ this, "/cmd/line-visible" };
	xf::PropertyIn<si::Angle>				cmd_heading_magnetic					{ this, "/cmd/heading-magnetic" };
	xf::PropertyIn<si::Angle>				cmd_track_magnetic						{ this, "/cmd/track-magnetic" };
	xf::PropertyIn<bool>					cmd_use_trk								{ this, "/cmd/use-trk" };
	xf::PropertyIn<si::Length>				target_altitude_reach_distance			{ this, "/target-altitude-reach-distance" };
	xf::PropertyIn<si::Angle>				orientation_heading_magnetic			{ this, "/orientation/heading-magnetic" };
	xf::PropertyIn<si::Angle>				orientation_heading_true				{ this, "/orientation/heading-true" };
	xf::PropertyIn<bool>					use_true_heading						{ this, "/use-true-heading" };
	xf::PropertyIn<si::Angle>				home_true_direction						{ this, "/home/true-direction" };
	xf::PropertyIn<bool>					home_track_visible						{ this, "/home/track-visible" };
	xf::PropertyIn<si::Length>				home_distance_vlos						{ this, "/home/distance/vlos" };
	xf::PropertyIn<si::Length>				home_distance_ground					{ this, "/home/distance/ground" };
	xf::PropertyIn<si::Length>				home_distance_vertical					{ this, "/home/distance/vertical" };
	xf::PropertyIn<si::Angle>				home_position_longitude					{ this, "/home/position/longitude" };
	xf::PropertyIn<si::Angle>				home_position_latitude					{ this, "/home/position/latitude" };
	xf::PropertyIn<si::Angle>				position_longitude						{ this, "/position/longitude" };
	xf::PropertyIn<si::Angle>				position_latitude						{ this, "/position/latitude" };
	xf::PropertyIn<std::string>				position_source							{ this, "/position/source" };
	xf::PropertyIn<bool>					track_visible							{ this, "/track/visible" };
	xf::PropertyIn<si::Angle>				track_lateral_magnetic					{ this, "/track/lateral-magnetic" };
	xf::PropertyIn<si::AngularVelocity>		track_lateral_rotation					{ this, "/track/lateral-rotation" };
	xf::PropertyIn<bool>					track_center_on_track					{ this, "/track/center-on-track" };
	xf::PropertyIn<bool>					course_visible							{ this, "/course/visible" };
	xf::PropertyIn<si::Angle>				course_setting_magnetic					{ this, "/course/setting-magnetic" };
	xf::PropertyIn<si::Angle>				course_deviation						{ this, "/course/deviation" };
	xf::PropertyIn<bool>					course_to_flag							{ this, "/course/to-flag" };
	xf::PropertyIn<std::string>				navaid_selected_reference				{ this, "/navaid/selected/reference" };
	xf::PropertyIn<std::string>				navaid_selected_identifier				{ this, "/navaid/selected/identifier" };
	xf::PropertyIn<si::Length>				navaid_selected_distance				{ this, "/navaid/selected/distance" };
	xf::PropertyIn<si::Time>				navaid_selected_eta						{ this, "/navaid/selected/eta" };
	xf::PropertyIn<si::Angle>				navaid_selected_course_magnetic			{ this, "/navaid/selected/course-magnetic" };
	xf::PropertyIn<int64_t>					navaid_left_type						{ this, "/navaid/left/type" };
	xf::PropertyIn<std::string>				navaid_left_reference					{ this, "/navaid/left/reference" };
	xf::PropertyIn<std::string>				navaid_left_identifier					{ this, "/navaid/left/identifier" };
	xf::PropertyIn<si::Length>				navaid_left_distance					{ this, "/navaid/left/distance" };
	xf::PropertyIn<si::Angle>				navaid_left_initial_bearing_magnetic	{ this, "/navaid/left/initial-bearing-magnetic" };
	xf::PropertyIn<int64_t>					navaid_right_type						{ this, "/navaid/right/type" };
	xf::PropertyIn<std::string>				navaid_right_reference					{ this, "/navaid/right/reference" };
	xf::PropertyIn<std::string>				navaid_right_identifier					{ this, "/navaid/right/identifier" };
	xf::PropertyIn<si::Length>				navaid_right_distance					{ this, "/navaid/right/distance" };
	xf::PropertyIn<si::Angle>				navaid_right_initial_bearing_magnetic	{ this, "/navaid/right/initial-bearing-magnetic" };
	xf::PropertyIn<si::Length>				navigation_required_performance			{ this, "/navigation/required-performance" };
	xf::PropertyIn<si::Length>				navigation_actual_performance			{ this, "/navigation/actual-performance" };
	xf::PropertyIn<si::Angle>				wind_from_magnetic						{ this, "/wind/from-magnetic" };
	xf::PropertyIn<si::Velocity>			wind_speed_tas							{ this, "/wind/speed-tas" };
	xf::PropertyIn<std::string>				localizer_id							{ this, "/localizer-id" };
	xf::PropertyIn<bool>					tcas_on									{ this, "/tcas/on" };
	xf::PropertyIn<si::Length>				tcas_range								{ this, "/tcas/range" };
	xf::PropertyIn<bool>					features_fix							{ this, "/features/fix" };
	xf::PropertyIn<bool>					features_vor							{ this, "/features/vor" };
	xf::PropertyIn<bool>					features_dme							{ this, "/features/dme" };
	xf::PropertyIn<bool>					features_ndb							{ this, "/features/ndb" };
	xf::PropertyIn<bool>					features_loc							{ this, "/features/loc" };
	xf::PropertyIn<bool>					features_arpt							{ this, "/features/arpt" };
};


class HSI: public xf::Instrument<HSI_IO>
{
	Q_OBJECT

  public:
	// Ctor
	HSI (std::unique_ptr<HSI_IO>, xf::WorkPerformer&, xf::NavaidStorage*, std::string const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	HSIWidget*	_hsi_widget;
};

#endif
