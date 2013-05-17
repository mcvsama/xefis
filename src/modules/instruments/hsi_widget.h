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

#ifndef XEFIS__MODULES__INSTRUMENTS__HSI_WIDGET_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__HSI_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QDateTime>
#include <QtGui/QColor>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument_widget.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/navaid_storage.h>
#include <xefis/core/work_performer.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/mutex.h>


using Xefis::NavaidStorage;

class HSIWidget: public Xefis::InstrumentWidget
{
  public:
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

  private:
	class Parameters
	{
	  public:
		DisplayMode				display_mode				= DisplayMode::Expanded;
		HeadingMode				heading_mode				= HeadingMode::Magnetic;
		Length					range						= 1_nm;
		Angle					magnetic_heading			= 0_deg;
		Angle					true_heading				= 0_deg;
		Angle					heading						= 0_deg; // Computed mag or true, depending on heading mode.
		bool					heading_visible				= false;
		Angle					rotation					= 0_deg;
		Angle					ap_magnetic_heading			= 0_deg;
		Angle					ap_heading					= 0_deg; // Computed mag or true, depending on heading mode.
		bool					ap_heading_visible			= false;
		bool					ap_track_visible			= false;
		Angle					magnetic_track				= 0_deg;
		Angle					true_track					= 0_deg; // Computed.
		Angle					track						= 0_deg; // Mag or true, depending on heading mode.
		bool					track_visible				= false;
		bool					display_track				= false;
		Angle					true_home_direction			= 0_deg;
		bool					home_direction_visible		= false;
		Length					distance_to_home_ground		= 0_nm;
		bool					dist_to_home_ground_visible	= false;
		Length					distance_to_home_vlos		= 0_nm;
		bool					dist_to_home_vlos_visible	= false;
		Speed					ground_speed				= 0_kt;
		bool					ground_speed_visible		= false;
		Speed					true_air_speed				= 0_kt;
		bool					true_air_speed_visible		= false;
		float					mach						= 0.f;
		bool					mach_visible				= false;
		Angle					track_lateral_delta			= 0_deg;
		bool					trend_vector_visible		= false;
		Length					trend_vector_lookahead		= 5_nm;
		Length					altitude_reach_distance		= 0_nm;
		bool					altitude_reach_visible		= false;
		Angle					wind_from_magnetic_heading	= 0_deg;
		Speed					wind_tas_speed				= 0_kt;
		bool					wind_information_visible	= false;
		LonLat					position					= { 0_deg, 0_deg };
		bool					position_valid				= false;
		bool					navaids_visible				= false;
		bool					vor_visible					= false;
		bool					dme_visible					= false;
		bool					ndb_visible					= false;
		bool					loc_visible					= false;
		bool					fix_visible					= false;
		QString					highlighted_loc;
		QString					positioning_hint;
		bool					positioning_hint_visible	= false;
		QDateTime				positioning_hint_ts;
		float					climb_glide_ratio			= 0.0;
		bool					climb_glide_ratio_visible	= false;
		bool					round_clip					= false;
	};

	class PaintWorkUnit:
		public InstrumentWidget::PaintWorkUnit,
		public Xefis::InstrumentAids
	{
		friend class HSIWidget;

	  public:
		PaintWorkUnit (HSIWidget*);

		~PaintWorkUnit() noexcept { }

		void
		set_navaid_storage (NavaidStorage*);

	  private:
		void
		pop_params() override;

		void
		resized() override;

		void
		paint (QImage&) override;

		void
		paint_aircraft (Painter&);

		void
		paint_hints (Painter&);

		void
		paint_ap_settings (Painter&);

		void
		paint_directions (Painter&);

		void
		paint_track (Painter&, bool paint_heading_triangle);

		void
		paint_altitude_reach (Painter&);

		void
		paint_trend_vector (Painter&);

		void
		paint_speeds_and_wind (Painter&);

		void
		paint_home_direction (Painter&);

		void
		paint_climb_glide_ratio (Painter&);

		void
		paint_range (Painter&);

		void
		paint_navaids (Painter&);

		void
		paint_locs (Painter&);

		/**
		 * Retrieve navaids from navaid storage for current aircraft
		 * position and populate _*_navs variables.
		 */
		void
		retrieve_navaids();

		/**
		 * Compute position where navaid should be drawn on map
		 * relative to the aircraft (assumes usage with aircraft-centered transform).
		 */
		QPointF
		get_navaid_xy (LonLat const& position);

		Length
		actual_trend_range() const;

		Length
		actual_trend_start() const;

		float
		nm_to_px (Length miles);

		bool
		is_newly_set (QDateTime const& timestamp, Time time = 10_s) const;

	  private:
		QDateTime				_current_datetime;
		NavaidStorage*			_navaid_storage				= nullptr;
		bool					_recalculation_needed		= false;
		float					_r;
		float					_q;
		TextPainter::Cache		_text_painter_cache;
		QTransform				_aircraft_center_transform;
		QTransform				_heading_transform;
		QTransform				_track_transform;
		QTransform				_rotation_transform;
		QTransform				_features_transform;
		QRectF					_map_clip_rect;
		QRectF					_trend_vector_clip_rect;
		QPainterPath			_inner_map_clip;
		QPainterPath			_outer_map_clip;
		QPen					_ndb_pen;
		QPen					_vor_pen;
		QPen					_dme_pen;
		QPen					_fix_pen;
		QPen					_lo_loc_pen;
		QPen					_hi_loc_pen;
		QFont					_radials_font;
		QPainterPath			_ndb_shape;
		QPolygonF				_dme_for_vor_shape;
		QPolygonF				_vor_shape;
		QPolygonF				_vortac_shape;
		QPolygonF				_aircraft_shape;
		QPolygonF				_ap_bug_shape;
		bool					_navs_retrieved				= false;
		LonLat					_navs_retrieve_position		= { 0_deg, 0_deg };
		Length					_navs_retrieve_range		= 0_nm;
		NavaidStorage::Navaids	_loc_navs;
		NavaidStorage::Navaids	_ndb_navs;
		NavaidStorage::Navaids	_vor_navs;
		NavaidStorage::Navaids	_dme_navs;
		NavaidStorage::Navaids	_fix_navs;
		Parameters				_params;
		Parameters				_params_next;
	};

  public:
	// Ctor
	HSIWidget (QWidget* parent, Xefis::WorkPerformer*);

	// Dtor
	~HSIWidget();

	/**
	 * Set reference to the nav storage, if you want navaids
	 * displayed on the HSI.
	 * Object must be live as long as HSIWidget is live.
	 * Pass nullptr to deassign.
	 */
	void
	set_navaid_storage (NavaidStorage*);

	/**
	 * Set HSI display mode.
	 * Default is Expanded.
	 */
	void
	set_display_mode (DisplayMode);

	/**
	 * Set navigation range.
	 */
	void
	set_range (Length miles);

	/**
	 * Set true heading value.
	 * Note that both magnetic and true heading must be set
	 * correctly for HSI to display navaids and scales properly.
	 */
	void
	set_true_heading (Angle);

	/**
	 * Set magnetic heading value.
	 * Note that both magnetic and true heading must be set
	 * correctly for HSI to display navaids and scales properly.
	 */
	void
	set_magnetic_heading (Angle);

	/**
	 * Select magnetic or true heading to display.
	 */
	void
	set_heading_mode (HeadingMode);

	/**
	 * Toggle heading scale visibility.
	 */
	void
	set_heading_visible (bool visible);

	/**
	 * Set A/P magnetic heading.
	 */
	void
	set_ap_magnetic_heading (Angle);

	/**
	 * Set A/P heading visibility.
	 */
	void
	set_ap_heading_visible (bool visible);

	/**
	 * Set A/P heading track visibility.
	 * Nominally turned on, when A/P is active.
	 */
	void
	set_ap_track_visible (bool visible);

	/**
	 * Set track magnetic heading.
	 */
	void
	set_magnetic_track (Angle);

	/**
	 * Set visibility of the track line.
	 */
	void
	set_track_visible (bool visible);

	/**
	 * Set display of track istead of heading.
	 */
	void
	set_display_track (bool track);

	/**
	 * Set true home (start position) direction.
	 */
	void
	set_true_home_direction (Angle direction);

	/**
	 * Set visibility of the home aids (direction, distance).
	 */
	void
	set_home_direction_visible (bool visible);

	/**
	 * Set ground distance to home.
	 */
	void
	set_ground_distance_to_home (Length ground_distance);

	/**
	 * Set visibility of ground distance to home.
	 */
	void
	set_ground_distance_to_home_visible (bool visible);

	/**
	 * Set VLOS (visual line of sight) distance to home.
	 */
	void
	set_vlos_distance_to_home (Length vlos_distance);

	/**
	 * Set visibility of VLOS distance to home.
	 */
	void
	set_vlos_distance_to_home_visible (bool visible);

	/**
	 * Set ground speed.
	 */
	void
	set_ground_speed (Speed);

	/**
	 * Toggle visibility of the ground speed.
	 */
	void
	set_ground_speed_visible (bool visible);

	/**
	 * Set true air speed.
	 */
	void
	set_true_air_speed (Speed);

	/**
	 * Toggle visibility of the true air speed.
	 */
	void
	set_true_air_speed_visible (bool visible);

	/**
	 * Set mach number indicator.
	 */
	void
	set_mach (float value);

	/**
	 * Set mach number indicator visibility.
	 */
	void
	set_mach_visible (bool visible);

	/**
	 * Set current position coordinates.
	 */
	void
	set_position (LonLat const&);

	/**
	 * Set position-valid flag.
	 */
	void
	set_position_valid (bool valid);

	/**
	 * Set positioning hint.
	 */
	void
	set_positioning_hint (QString const& hint);

	/**
	 * Set positioning hint visibility.
	 */
	void
	set_positioning_hint_visible (bool visible);

	/**
	 * Set track estimation in degrees per mile flown.
	 * Positive degrees means turning to the right, negative - to the left.
	 */
	void
	set_track_trend (Angle degrees_per_mile);

	/**
	 * Set track estimation visibility.
	 */
	void
	set_trend_vector_visible (bool visible);

	/**
	 * Set track estimation lookahead in nautical miles.
	 */
	void
	set_trend_vector_lookahead (Length lookahead);

	/**
	 * Set position of desired-altitude-reach-point.
	 */
	void
	set_altitude_reach_distance (Length);

	/**
	 * Set visibility of the desired-altitude-reach-point curve.
	 */
	void
	set_altitude_reach_visible (bool visible);

	/**
	 * Set wind information.
	 * \param	wind_magnetic_heading Direction from which wind comes.
	 */
	void
	set_wind_information (Angle wind_from_magnetic_heading, Speed wind_tas_speed);

	/**
	 * Set wind information visibility.
	 */
	void
	set_wind_information_visible (bool visible);

	/**
	 * Set navaids visibility.
	 */
	void
	set_navaids_visible (bool visible);

	/**
	 * Set visibility of VORs.
	 * They will be visible only if also general navaids visibility
	 * is turned on with set_navaids_visible().
	 */
	void
	set_vor_visible (bool visible);

	/**
	 * Set visibility of DMEs.
	 * They will be visible only if also general navaids visibility
	 * is turned on with set_navaids_visible().
	 */
	void
	set_dme_visible (bool visible);

	/**
	 * Set visibility of NDBs.
	 * They will be visible only if also general navaids visibility
	 * is turned on with set_navaids_visible().
	 */
	void
	set_ndb_visible (bool visible);

	/**
	 * Set visibility of localisers.
	 * They will be visible only if also general navaids visibility
	 * is turned on with set_navaids_visible().
	 */
	void
	set_loc_visible (bool visible);

	/**
	 * Set visibility of the Fixes.
	 * They will be visible only if also general navaids visibility
	 * is turned on with set_navaids_visible().
	 */
	void
	set_fix_visible (bool visible);

	/**
	 * Select LOC to highlight by its identifier string.
	 * Highlighted LOC will be shown in different color.
	 */
	void
	set_highlighted_loc (QString const& identifier);

	/**
	 * Reset highlighted localizer.
	 */
	void
	reset_highlighted_loc();

	/**
	 * Set climb-glide ratio value.
	 */
	void
	set_climb_glide_ratio (float ratio);

	/**
	 * Set climb-glide indicator visibility.
	 */
	void
	set_climb_glide_ratio_visible (bool visible);

	/**
	 * Set ground features clipping to the directions circle.
	 */
	void
	set_rounded_clipping (bool enabled);

  private:
	// InstrumentWidget API
	void
	push_params() override;

  private:
	PaintWorkUnit	_paint_work_unit;
	Parameters		_params;
};


inline void
HSIWidget::PaintWorkUnit::set_navaid_storage (NavaidStorage* navaid_storage)
{
	_navaid_storage = navaid_storage;
}


inline QPointF
HSIWidget::PaintWorkUnit::get_navaid_xy (LonLat const& position)
{
	QPointF navaid_pos = EARTH_MEAN_RADIUS.nm() * position.rotated (_params.position).project_flat();
	return _features_transform.map (QPointF (nm_to_px (1_nm * navaid_pos.x()), nm_to_px (1_nm * navaid_pos.y())));
}


inline Length
HSIWidget::PaintWorkUnit::actual_trend_range() const
{
	float limit = 0;
	switch (_params.display_mode)
	{
		case DisplayMode::Expanded:
			limit = 0.13f;
			break;
		case DisplayMode::Rose:
			limit = 0.18f;
			break;
		case DisplayMode::Auxiliary:
			limit = 0.36f;
			break;
	}
	return std::min (_params.trend_vector_lookahead, limit * _params.range);
}


inline Length
HSIWidget::PaintWorkUnit::actual_trend_start() const
{
	switch (_params.display_mode)
	{
		case DisplayMode::Expanded:
			return 0.015f * _params.range;
		case DisplayMode::Rose:
			return 0.030f * _params.range;
		case DisplayMode::Auxiliary:
			return 0.0375f * _params.range;
	}
	return 0_nm;
}


inline float
HSIWidget::PaintWorkUnit::nm_to_px (Length miles)
{
	return miles / _params.range * _r;
}


inline bool
HSIWidget::PaintWorkUnit::is_newly_set (QDateTime const& timestamp, Time time) const
{
	return timestamp.secsTo (_current_datetime) < time.s();
}


inline void
HSIWidget::set_navaid_storage (NavaidStorage* navaid_storage)
{
	_paint_work_unit.set_navaid_storage (navaid_storage);
	request_repaint();
}


inline void
HSIWidget::set_display_mode (DisplayMode display_mode)
{
	_params.display_mode = display_mode;
	_paint_work_unit._recalculation_needed = true;
	request_repaint();
}


inline void
HSIWidget::set_range (Length miles)
{
	_params.range = miles;
	request_repaint();
}


inline void
HSIWidget::set_true_heading (Angle degrees)
{
	_params.true_heading = degrees;
	request_repaint();
}


inline void
HSIWidget::set_magnetic_heading (Angle degrees)
{
	_params.magnetic_heading = degrees;
	request_repaint();
}


inline void
HSIWidget::set_heading_mode (HeadingMode mode)
{
	_params.heading_mode = mode;
	request_repaint();
}


inline void
HSIWidget::set_heading_visible (bool visible)
{
	_params.heading_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_ap_magnetic_heading (Angle heading)
{
	_params.ap_magnetic_heading = heading;
	request_repaint();
}


inline void
HSIWidget::set_ap_heading_visible (bool visible)
{
	_params.ap_heading_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_ap_track_visible (bool visible)
{
	_params.ap_track_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_magnetic_track (Angle track)
{
	_params.magnetic_track = track;
	request_repaint();
}


inline void
HSIWidget::set_track_visible (bool visible)
{
	_params.track_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_display_track (bool track)
{
	_params.display_track = track;
	request_repaint();
}


inline void
HSIWidget::set_true_home_direction (Angle direction)
{
	_params.true_home_direction = direction;
	request_repaint();
}


inline void
HSIWidget::set_home_direction_visible (bool visible)
{
	_params.home_direction_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_ground_distance_to_home (Length ground_distance)
{
	_params.distance_to_home_ground = ground_distance;
	request_repaint();
}


inline void
HSIWidget::set_ground_distance_to_home_visible (bool visible)
{
	_params.dist_to_home_ground_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_vlos_distance_to_home (Length vlos_distance)
{
	_params.distance_to_home_vlos = vlos_distance;
	request_repaint();
}


inline void
HSIWidget::set_vlos_distance_to_home_visible (bool visible)
{
	_params.dist_to_home_vlos_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_ground_speed (Speed ground_speed)
{
	_params.ground_speed = ground_speed;
	request_repaint();
}


inline void
HSIWidget::set_ground_speed_visible (bool visible)
{
	_params.ground_speed_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_true_air_speed (Speed true_air_speed)
{
	_params.true_air_speed = true_air_speed;
	request_repaint();
}


inline void
HSIWidget::set_true_air_speed_visible (bool visible)
{
	_params.true_air_speed_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_mach (float value)
{
	_params.mach = value;
	request_repaint();
}


inline void
HSIWidget::set_mach_visible (bool visible)
{
	_params.mach_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_position (LonLat const& position)
{
	_params.position = position;
	request_repaint();
}


inline void
HSIWidget::set_position_valid (bool valid)
{
	_params.position_valid = valid;
	request_repaint();
}


inline void
HSIWidget::set_positioning_hint (QString const& hint)
{
	if (_params.positioning_hint != hint)
		_params.positioning_hint_ts = QDateTime::currentDateTime();
	_params.positioning_hint = hint;
	request_repaint();
}


inline void
HSIWidget::set_positioning_hint_visible (bool visible)
{
	if (_params.positioning_hint_visible != visible)
		_params.positioning_hint_ts = QDateTime::currentDateTime();
	_params.positioning_hint_visible = visible;
}


inline void
HSIWidget::set_track_trend (Angle degrees_per_mile)
{
	_params.track_lateral_delta = degrees_per_mile;
	request_repaint();
}


inline void
HSIWidget::set_trend_vector_visible (bool visible)
{
	_params.trend_vector_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_trend_vector_lookahead (Length lookahead)
{
	_params.trend_vector_lookahead = lookahead;
	request_repaint();
}


inline void
HSIWidget::set_altitude_reach_distance (Length distance)
{
	_params.altitude_reach_distance = distance;
	request_repaint();
}


inline void
HSIWidget::set_altitude_reach_visible (bool visible)
{
	_params.altitude_reach_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_wind_information (Angle wind_from_magnetic_heading, Speed wind_tas_speed)
{
	_params.wind_from_magnetic_heading = wind_from_magnetic_heading;
	_params.wind_tas_speed = wind_tas_speed;
	request_repaint();
}


inline void
HSIWidget::set_wind_information_visible (bool visible)
{
	_params.wind_information_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_navaids_visible (bool visible)
{
	_params.navaids_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_vor_visible (bool visible)
{
	_params.vor_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_dme_visible (bool visible)
{
	_params.dme_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_ndb_visible (bool visible)
{
	_params.ndb_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_loc_visible (bool visible)
{
	_params.loc_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_fix_visible (bool visible)
{
	_params.fix_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_highlighted_loc (QString const& identifier)
{
	_params.highlighted_loc = identifier;
	request_repaint();
}


inline void
HSIWidget::reset_highlighted_loc()
{
	_params.highlighted_loc = "";
	request_repaint();
}


inline void
HSIWidget::set_climb_glide_ratio (float ratio)
{
	_params.climb_glide_ratio = ratio;
	request_repaint();
}


inline void
HSIWidget::set_climb_glide_ratio_visible (bool visible)
{
	_params.climb_glide_ratio_visible = visible;
	request_repaint();
}


inline void
HSIWidget::set_rounded_clipping (bool enabled)
{
	_params.round_clip = enabled;
	request_repaint();
}


inline void
HSIWidget::push_params()
{
	_paint_work_unit._params_next = _params;
}

#endif

