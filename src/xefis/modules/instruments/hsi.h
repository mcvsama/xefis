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
#include <array>
#include <cstddef>
#include <future>

// Neutrino:
#include <neutrino/synchronized.h>

// Qt:
#include <QPainterPath>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/instrument.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/core/xefis.h>
#include <xefis/support/earth/navigation/navaid_storage.h>
#include <xefis/support/instrument/instrument_support.h>
#include <xefis/utility/event_timestamper.h>
#include <xefis/utility/temporal.h>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


// TODO handle nans
namespace hsi {

enum class DisplayMode
{
	/**
	 * Map is expanded on the front of the aircraft.
	 */
	Expanded,

	/**
	 * Aircraft is shown in the center of the widget. Map covers all directions
	 * of the aircraft. This is useful mode to use with VOR/ILS navigation.
	 */
	Rose,

	/**
	 * Similar to the Expanded mode, but less information is displayed.
	 * This is useful mode to be displayed under the EFIS widget.
	 */
	Auxiliary
};


enum class HeadingMode
{
	/**
	 * Display magnetic heading on scale.
	 */
	Magnetic,

	/**
	 * Display true heading on scale.
	 */
	True
};


enum class NavType
{
	A,
	B
};


static constexpr std::string_view	kDisplayMode_Expanded	= "expanded";
static constexpr std::string_view	kDisplayMode_Rose		= "rose";
static constexpr std::string_view	kDisplayMode_Auxiliary	= "auxiliary";

static constexpr std::string_view	kHeadingMode_Magnetic	= "MAG";
static constexpr std::string_view	kHeadingMode_True		= "TRU";

static constexpr std::string_view	kNavType_A				= "A";
static constexpr std::string_view	kNavType_B				= "B";


constexpr std::string_view
to_string (DisplayMode mode)
{
	switch (mode)
	{
		case DisplayMode::Expanded:		return kDisplayMode_Expanded;
		case DisplayMode::Rose:			return kDisplayMode_Rose;
		case DisplayMode::Auxiliary:	return kDisplayMode_Auxiliary;
	}

	return "";
}


constexpr std::string_view
to_string (HeadingMode mode)
{
	switch (mode)
	{
		case HeadingMode::Magnetic:		return kHeadingMode_Magnetic;
		case HeadingMode::True:			return kHeadingMode_True;
	}

	return "";
}


constexpr std::string_view
to_string (NavType nav_type)
{
	switch (nav_type)
	{
		case NavType::A:	return kNavType_A;
		case NavType::B:	return kNavType_B;
	}

	return "";
}


inline void
parse (std::string_view const& str, DisplayMode& display_mode)
{
	if (str == kDisplayMode_Expanded)
		display_mode = DisplayMode::Expanded;
	else if (str == kDisplayMode_Rose)
		display_mode = DisplayMode::Rose;
	else if (str == kDisplayMode_Rose)
		display_mode = DisplayMode::Auxiliary;
}


inline void
parse (std::string_view const& str, HeadingMode& heading_mode)
{
	if (str == kHeadingMode_Magnetic)
		heading_mode = HeadingMode::Magnetic;
	else if (str == kHeadingMode_True)
		heading_mode = HeadingMode::True;
}


inline void
parse (std::string_view const& str, NavType& nav_type)
{
	if (str == kNavType_A)
		nav_type = NavType::A;
	else if (str == kNavType_B)
		nav_type = NavType::B;
}

} // namespace hsi


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
	// How big should be dots on the radio range heat map? 1.0 means 1x1 hardware pixel. Value 2…3 is recommended.
	xf::Setting<double>						radio_range_pattern_scale				{ this, "radio_range_pattern_scale", 2.5 };

	/*
	 * Input
	 */

	xf::ModuleIn<hsi::DisplayMode>			display_mode							{ this, "display-mode", hsi::DisplayMode::Expanded };
	xf::ModuleIn<si::Length>				range									{ this, "range", 5_nmi };
	xf::ModuleIn<si::Velocity>				speed_gs								{ this, "speeds/gs" };
	xf::ModuleIn<si::Velocity>				speed_tas								{ this, "speeds/tas" };
	xf::ModuleIn<bool>						cmd_visible								{ this, "cmd/visible" };
	xf::ModuleIn<bool>						cmd_line_visible						{ this, "cmd/line-visible" };
	xf::ModuleIn<si::Angle>					cmd_heading_magnetic					{ this, "cmd/heading-magnetic" };
	xf::ModuleIn<si::Angle>					cmd_track_magnetic						{ this, "cmd/track-magnetic" };
	xf::ModuleIn<bool>						cmd_use_trk								{ this, "cmd/use-trk" };
	xf::ModuleIn<si::Length>				target_altitude_reach_distance			{ this, "target-altitude-reach-distance" };
	xf::ModuleIn<si::Angle>					orientation_heading_magnetic			{ this, "orientation/heading-magnetic" };
	xf::ModuleIn<si::Angle>					orientation_heading_true				{ this, "orientation/heading-true" };
	xf::ModuleIn<hsi::HeadingMode>			heading_mode							{ this, "heading-mode" };
	xf::ModuleIn<si::Angle>					home_true_direction						{ this, "home/true-direction" };
	xf::ModuleIn<bool>						home_track_visible						{ this, "home/track-visible" };
	xf::ModuleIn<si::Length>				home_distance_vlos						{ this, "home/distance/vlos" };
	xf::ModuleIn<si::Length>				home_distance_ground					{ this, "home/distance/ground" };
	xf::ModuleIn<si::Length>				home_distance_vertical					{ this, "home/distance/vertical" };
	xf::ModuleIn<si::Angle>					home_position_longitude					{ this, "home/position/longitude" };
	xf::ModuleIn<si::Angle>					home_position_latitude					{ this, "home/position/latitude" };
	xf::ModuleIn<si::Angle>					position_longitude						{ this, "position/longitude" };
	xf::ModuleIn<si::Angle>					position_latitude						{ this, "position/latitude" };
	xf::ModuleIn<std::string>				position_source							{ this, "position/source" };
	xf::ModuleIn<bool>						track_visible							{ this, "track/visible" };
	xf::ModuleIn<si::Angle>					track_lateral_magnetic					{ this, "track/lateral-magnetic" };
	xf::ModuleIn<si::AngularVelocity>		track_lateral_rotation					{ this, "track/lateral-rotation" };
	xf::ModuleIn<bool>						track_center_on_track					{ this, "track/center-on-track" };
	xf::ModuleIn<bool>						course_visible							{ this, "course/visible" };
	xf::ModuleIn<si::Angle>					course_setting_magnetic					{ this, "course/setting-magnetic" };
	xf::ModuleIn<si::Angle>					course_deviation						{ this, "course/deviation" };
	xf::ModuleIn<bool>						course_to_flag							{ this, "course/to-flag" };
	xf::ModuleIn<std::string>				navaid_selected_reference				{ this, "navaid/selected/reference" };
	xf::ModuleIn<std::string>				navaid_selected_identifier				{ this, "navaid/selected/identifier" };
	xf::ModuleIn<si::Length>				navaid_selected_distance				{ this, "navaid/selected/distance" };
	xf::ModuleIn<si::Time>					navaid_selected_eta						{ this, "navaid/selected/eta" };
	xf::ModuleIn<si::Angle>					navaid_selected_course_magnetic			{ this, "navaid/selected/course-magnetic" };
	xf::ModuleIn<hsi::NavType>				navaid_left_type						{ this, "navaid/left/type" };
	xf::ModuleIn<std::string>				navaid_left_reference					{ this, "navaid/left/reference" };
	xf::ModuleIn<std::string>				navaid_left_identifier					{ this, "navaid/left/identifier" };
	xf::ModuleIn<si::Length>				navaid_left_distance					{ this, "navaid/left/distance" };
	xf::ModuleIn<si::Angle>					navaid_left_initial_bearing_magnetic	{ this, "navaid/left/initial-bearing-magnetic" };
	xf::ModuleIn<hsi::NavType>				navaid_right_type						{ this, "navaid/right/type" };
	xf::ModuleIn<std::string>				navaid_right_reference					{ this, "navaid/right/reference" };
	xf::ModuleIn<std::string>				navaid_right_identifier					{ this, "navaid/right/identifier" };
	xf::ModuleIn<si::Length>				navaid_right_distance					{ this, "navaid/right/distance" };
	xf::ModuleIn<si::Angle>					navaid_right_initial_bearing_magnetic	{ this, "navaid/right/initial-bearing-magnetic" };
	xf::ModuleIn<si::Length>				navigation_required_performance			{ this, "navigation/required-performance" };
	xf::ModuleIn<si::Length>				navigation_actual_performance			{ this, "navigation/actual-performance" };
	xf::ModuleIn<si::Angle>					wind_from_magnetic						{ this, "wind/from-magnetic" };
	xf::ModuleIn<si::Velocity>				wind_speed_tas							{ this, "wind/speed-tas" };
	xf::ModuleIn<std::string>				localizer_id							{ this, "localizer-id" };
	xf::ModuleIn<bool>						tcas_on									{ this, "tcas/on" };
	xf::ModuleIn<si::Length>				tcas_range								{ this, "tcas/range" };
	xf::ModuleIn<bool>						features_fix							{ this, "features/fix" };
	xf::ModuleIn<bool>						features_vor							{ this, "features/vor" };
	xf::ModuleIn<bool>						features_dme							{ this, "features/dme" };
	xf::ModuleIn<bool>						features_ndb							{ this, "features/ndb" };
	xf::ModuleIn<bool>						features_loc							{ this, "features/loc" };
	xf::ModuleIn<bool>						features_arpt							{ this, "features/arpt" };
	xf::ModuleIn<si::Angle>					flight_range_warning_longitude			{ this, "range/warning/longitude" };
	xf::ModuleIn<si::Angle>					flight_range_warning_latitude			{ this, "range/warning/latitude" };
	xf::ModuleIn<si::Length>				flight_range_warning_radius				{ this, "range/warning/radius" };
	xf::ModuleIn<si::Angle>					flight_range_critical_longitude			{ this, "range/critical/longitude" };
	xf::ModuleIn<si::Angle>					flight_range_critical_latitude			{ this, "range/critical/latitude" };
	xf::ModuleIn<si::Length>				flight_range_critical_radius			{ this, "range/critical/radius" };
	xf::ModuleIn<si::Angle>					radio_position_longitude				{ this, "radio-range/position/longitude" };
	xf::ModuleIn<si::Angle>					radio_position_latitude					{ this, "radio-range/position/latitude" };
	xf::ModuleIn<si::Length>				radio_range_warning						{ this, "radio-range/radius.warning" };
	xf::ModuleIn<si::Length>				radio_range_critical					{ this, "radio-range/radius.critical" };
};


namespace hsi_detail {

class CircularArea
{
  public:
	si::LonLat	center;
	si::Length	radius;
};


class Parameters
{
  public:
	si::Time								update_time								{ 0_s };
	hsi::DisplayMode						display_mode							{ hsi::DisplayMode::Expanded };
	hsi::HeadingMode						heading_mode							{ hsi::HeadingMode::Magnetic };
	si::Length								range									{ 1_nmi };
	std::optional<si::Angle>				heading_magnetic;
	std::optional<si::Angle>				heading_true;
	bool									ap_visible								{ false };
	bool									ap_line_visible							{ false };
	std::optional<si::Angle>				ap_heading_magnetic;
	std::optional<si::Angle>				ap_track_magnetic;
	std::optional<bool>						ap_use_trk;
	bool									track_visible							{ false };
	std::optional<si::Angle>				track_magnetic;
	bool									course_visible							{ false };
	std::optional<si::Angle>				course_setting_magnetic;
	std::optional<si::Angle>				course_deviation;
	std::optional<bool>						course_to_flag;
	QString									navaid_selected_reference;
	QString									navaid_selected_identifier;
	std::optional<si::Length>				navaid_selected_distance;
	std::optional<si::Time>					navaid_selected_eta;
	std::optional<si::Angle>				navaid_selected_course_magnetic;
	hsi::NavType							navaid_left_type						{ hsi::NavType::A };
	QString									navaid_left_reference;
	QString									navaid_left_identifier;
	std::optional<si::Length>				navaid_left_distance;
	std::optional<si::Angle>				navaid_left_initial_bearing_magnetic;
	hsi::NavType							navaid_right_type						{ hsi::NavType::A };
	QString									navaid_right_reference;
	QString									navaid_right_identifier;
	std::optional<si::Length>				navaid_right_distance;
	std::optional<si::Angle>				navaid_right_initial_bearing_magnetic;
	std::optional<si::Length>				navigation_required_performance;
	std::optional<si::Length>				navigation_actual_performance;
	bool									center_on_track							{ false };
	bool									home_track_visible						{ false };
	std::optional<si::Angle>				true_home_direction;
	std::optional<si::Length>				dist_to_home_ground;
	std::optional<si::Length>				dist_to_home_vlos;
	std::optional<si::Length>				dist_to_home_vert;
	std::optional<si::LonLat>				home;
	std::optional<si::Speed>				ground_speed;
	std::optional<si::Speed>				true_air_speed;
	std::optional<si::AngularVelocity>		track_lateral_rotation;
	std::optional<si::Length>				altitude_reach_distance;
	std::optional<si::Angle>				wind_from_magnetic_heading;
	std::optional<si::Velocity>				wind_tas_speed;
	std::optional<si::LonLat>				position;
	bool									navaids_visible							{ false };
	bool									fix_visible								{ false };
	bool									vor_visible								{ false };
	bool									dme_visible								{ false };
	bool									ndb_visible								{ false };
	bool									loc_visible								{ false };
	bool									arpt_visible							{ false };
	QString									highlighted_loc;
	xf::Temporal<std::optional<QString>>	positioning_hint;
	std::optional<bool>						tcas_on;
	std::optional<si::Length>				tcas_range;
	si::Length								arpt_runways_range_threshold;
	si::Length								arpt_map_range_threshold;
	si::Length								arpt_runway_extension_length;
	std::array<si::Time, 3>					trend_vector_durations;
	std::array<si::Length, 3>				trend_vector_min_ranges;
	si::Length								trend_vector_max_range;
	double									radio_range_pattern_scale;
	bool									round_clip								{ false };
	std::optional<CircularArea>				flight_range_warning;
	std::optional<CircularArea>				flight_range_critical;
	std::optional<si::LonLat>				radio_position;
	std::optional<si::Length>				radio_range_warning;
	std::optional<si::Length>				radio_range_critical;

  public:
	/**
	 * Sanitize all parameters.
	 */
	void
	sanitize();
};


/**
 * Stuff in this class gets recomputed when widget is resized.
 */
struct ResizeCache
{
	float			r;
	float			q;
	float			vmargin;
	float			hmargin;
	QTransform		aircraft_center_transform;
	QRectF			trend_vector_clip_rect;
	QRectF			map_clip_rect;
	QPainterPath	inner_map_clip;
	QPainterPath	outer_map_clip;
	QFont			radials_font;
	QPen			lo_loc_pen;
	QPen			hi_loc_pen;
	QPen			ndb_pen;
	QPen			vor_pen;
	QPen			dme_pen;
	QPen			fix_pen;
	QPen			arpt_pen;
	QPen			home_pen;
	QPolygonF		dme_for_vor_shape;
	QPolygonF		vor_shape;
	QPolygonF		vortac_shape;
	QPolygonF		home_shape;
	QPolygonF		aircraft_shape;
	QPolygonF		ap_bug_shape;
	xf::Shadow		black_shadow;
	QImage			radio_range_heat_map;
};


/**
 * Navaids retrieved for given aircraft position and HSI range setting.
 */
struct CurrentNavaids
{
	xf::NavaidStorage::Navaids	fix_navs;
	xf::NavaidStorage::Navaids	vor_navs;
	xf::NavaidStorage::Navaids	dme_navs;
	xf::NavaidStorage::Navaids	ndb_navs;
	xf::NavaidStorage::Navaids	loc_navs;
	xf::NavaidStorage::Navaids	arpt_navs;

	bool						retrieved			{ false };
	si::LonLat					retrieve_position	{ 0_deg, 0_deg };
	si::Length					retrieve_range		{ 0_nmi };
};


struct Mutable
{
	hsi::DisplayMode	prev_display_mode	{ hsi::DisplayMode::Expanded };
	si::Length			prev_range			{ 0_nmi };
};


class PaintingWork
{
  public:
	// Ctor
	explicit
	PaintingWork (xf::PaintRequest const&, xf::InstrumentSupport const&, xf::NavaidStorage const&, Parameters const&, ResizeCache&, CurrentNavaids&, Mutable&, xf::Logger const&);

	void
	paint();

  private:
	void
	paint_aircraft();

	void
	paint_navperf();

	void
	paint_hints();

	void
	paint_ap_settings();

	void
	paint_directions();

	void
	paint_track (bool paint_heading_triangle);

	void
	paint_altitude_reach();

	void
	paint_trend_vector();

	void
	paint_speeds_and_wind();

	void
	paint_home_direction();

	void
	paint_course();

	void
	paint_selected_navaid_info();

	void
	paint_tcas_and_navaid_info();

	void
	paint_pointers();

	void
	paint_range();

	void
	paint_navaids();

	void
	paint_radio_range_map();

	void
	paint_flight_ranges();

	void
	paint_circle (CircularArea const&, QColor color);

	void
	paint_locs();

	void
	paint_tcas();

	/**
	 * Paint radio range heat map into cached canvas.
	 * Needs update after each range parameter change.
	 */
	void
	update_radio_range_heat_map();

	/**
	 * Retrieve navaids from navaid storage for current aircraft
	 * position and populate _*_navs variables.
	 */
	void
	retrieve_navaids();

	/**
	 * Compute position where a feature should be drawn on map
	 * relative to the aircraft (assumes usage with aircraft-centered transform).
	 */
	QPointF
	get_feature_xy (si::LonLat const& navaid_position) const;

	/**
	 * Trend vector range.
	 */
	si::Length
	actual_trend_range() const;

	/**
	 * Gap between lines on trend vector.
	 */
	si::Length
	trend_gap() const;

	/**
	 * Time gap between lines on trend vector.
	 */
	si::Time
	trend_time_gap() const;

	float
	to_px (si::Length miles) const;

  private:
	xf::Logger const&						_logger;
	xf::PaintRequest const&					_paint_request;
	xf::NavaidStorage const&				_navaid_storage;
	Parameters const&						_p;
	ResizeCache&							_c;
	CurrentNavaids&							_current_navaids;
	Mutable&								_mutable;

	xf::InstrumentPainter					_painter;
	std::shared_ptr<xf::InstrumentAids>		_aids_ptr;
	xf::InstrumentAids&						_aids;

	std::optional<si::Angle>				_heading;						// Computed mag or true, depending on heading mode.
	std::optional<si::Angle>				_ap_bug_magnetic;				// Computed mag or true, depending on heading mode.
	std::optional<bool>						_ap_use_trk;
	std::optional<si::Angle>				_course_heading;				// Computed mag or true, depending on heading mode.
	std::optional<si::Angle>				_track_true;					// Computed.
	std::optional<si::Angle>				_track;							// Mag or true, depending on heading mode.
	std::optional<si::Angle>				_rotation;
	si::Time								_positioning_hint_changed_ts	{ 0_s };
	bool									_navaid_selected_visible		{ false };
	bool									_navaid_left_visible			{ false };
	bool									_navaid_right_visible			{ false };
	QTransform								_heading_transform;
	// TRK/HDG transform, depending if HDG or TRK is selected:
	QTransform								_rotation_transform;
	QTransform								_track_transform;
	// Transform for ground objects:
	QTransform								_features_transform;
	// Transform used for VOR/ADF pointers, that are represented by magnetic heading:
	QTransform								_pointers_transform;
};

} // namespace hsi_detail


class HSI: public xf::Instrument<HSI_IO>
{
  private:
	static constexpr char kLoggerScope[] = "mod::AirDataComputer";

  public:
	// Ctor
	HSI (std::unique_ptr<HSI_IO>, xf::Graphics const&, xf::NavaidStorage const&, xf::Logger const&, std::string_view const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

	// Instrument API
	std::packaged_task<void()>
	paint (xf::PaintRequest) const override;

  private:
	xf::Logger												_logger;
	xf::NavaidStorage const&								_navaid_storage;
	xf::InstrumentSupport									_instrument_support;
	xf::Synchronized<hsi_detail::Parameters> mutable		_parameters;
	xf::Synchronized<hsi_detail::ResizeCache> mutable		_resize_cache;
	xf::Synchronized<hsi_detail::CurrentNavaids> mutable	_current_navaids;
	xf::Synchronized<hsi_detail::Mutable> mutable			_mutable;
};

#endif

