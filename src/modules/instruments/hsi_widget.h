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

	class Parameters
	{
		friend class HSIWidget;
		friend class PaintWorkUnit;

	  public:
		DisplayMode			display_mode				= DisplayMode::Expanded;
		HeadingMode			heading_mode				= HeadingMode::Magnetic;
		Length				range						= 1_nm;
		bool				heading_visible				= false;
		Angle				heading_magnetic			= 0_deg;
		Angle				heading_true				= 0_deg;
		bool				ap_heading_visible			= false;
		bool				ap_track_visible			= false;
		Angle				ap_magnetic_heading			= 0_deg;
		bool				track_visible				= false;
		Angle				track_magnetic				= 0_deg;
		bool				course_visible				= false;
		Optional<Angle>		course_setting_magnetic;
		Optional<Angle>		course_deviation;
		Optional<bool>		course_to_flag;
		bool				navaid_visible;
		QString				navaid_identifier;
		QString				navaid_reference;
		Optional<Length>	navaid_distance;
		Optional<Time>		navaid_eta;
		Optional<Angle>		navaid_course_magnetic;
		bool				navaid_left_visible;
		QString				navaid_left_reference;
		QString				navaid_left_identifier;
		Optional<Length>	navaid_left_distance;
		Optional<Angle>		navaid_left_reciprocal_magnetic;
		bool				navaid_right_visible;
		QString				navaid_right_reference;
		QString				navaid_right_identifier;
		Optional<Length>	navaid_right_distance;
		Optional<Angle>		navaid_right_reciprocal_magnetic;
		bool				center_on_track				= false;
		bool				home_direction_visible		= false;
		bool				home_track_visible			= false;
		Angle				true_home_direction			= 0_deg;
		bool				dist_to_home_ground_visible	= false;
		Length				dist_to_home_ground			= 0_nm;
		bool				dist_to_home_vlos_visible	= false;
		Length				dist_to_home_vlos			= 0_nm;
		bool				dist_to_home_vert_visible	= false;
		Length				dist_to_home_vert			= 0_nm;
		Optional<LonLat>	home;
		bool				ground_speed_visible		= false;
		Speed				ground_speed				= 0_kt;
		bool				true_air_speed_visible		= false;
		Speed				true_air_speed				= 0_kt;
		bool				trend_vector_visible		= false;
		Angle				track_lateral_delta			= 0_deg;
		Length				trend_vector_lookahead		= 5_nm;
		bool				altitude_reach_visible		= false;
		Length				altitude_reach_distance		= 0_nm;
		bool				wind_information_visible	= false;
		Angle				wind_from_magnetic_heading	= 0_deg;
		Speed				wind_tas_speed				= 0_kt;
		bool				position_valid				= false;
		Optional<LonLat>	position;
		bool				navaids_visible				= false;
		bool				vor_visible					= false;
		bool				dme_visible					= false;
		bool				ndb_visible					= false;
		bool				loc_visible					= false;
		bool				fix_visible					= false;
		QString				highlighted_loc;
		bool				positioning_hint_visible	= false;
		QString				positioning_hint;
		Optional<bool>		tcas_on;
		Optional<Length>	tcas_range;
		bool				round_clip					= false;

	  private:
		/**
		 * Sanitize all parameters.
		 */
		void
		sanitize();
	};

  private:
	class LocalParameters
	{
	  public:
		Angle			heading						= 0_deg; // Computed mag or true, depending on heading mode.
		Angle			ap_heading					= 0_deg; // Computed mag or true, depending on heading mode.
		Angle			course_heading				= 0_deg; // Computed mag or true, depending on heading mode.
		Angle			track_true					= 0_deg; // Computed.
		Angle			track						= 0_deg; // Mag or true, depending on heading mode.
		Angle			rotation					= 0_deg;
		QDateTime		positioning_hint_ts			= QDateTime::fromTime_t (0);
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
		paint_aircraft (Xefis::Painter&);

		void
		paint_hints (Xefis::Painter&);

		void
		paint_ap_settings (Xefis::Painter&);

		void
		paint_directions (Xefis::Painter&);

		void
		paint_track (Xefis::Painter&, bool paint_heading_triangle);

		void
		paint_altitude_reach (Xefis::Painter&);

		void
		paint_trend_vector (Xefis::Painter&);

		void
		paint_speeds_and_wind (Xefis::Painter&);

		void
		paint_home_direction (Xefis::Painter&);

		void
		paint_course (Xefis::Painter&);

		void
		paint_selected_navaid_info();

		void
		paint_navaid_info();

		void
		paint_pointers (Xefis::Painter&);

		void
		paint_range (Xefis::Painter&);

		void
		paint_navaids (Xefis::Painter&);

		void
		paint_locs (Xefis::Painter&);

		void
		paint_tcas();

		void
		paint_bottom_text (bool left, unsigned int line_from_bottom, QColor, QString text);

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
		bool					_recalculation_needed		= true;
		float					_r;
		float					_q;
		float					_margin;
		QTransform				_aircraft_center_transform;
		QTransform				_heading_transform;
		QTransform				_track_transform;
		// TRK/HDG transform, depending if HDG or TRK is selected:
		QTransform				_rotation_transform;
		// Transform for ground objects:
		QTransform				_features_transform;
		// Transform used for VOR/ADF pointers, that are represented by magnetic heading:
		QTransform				_pointers_transform;
		QRectF					_map_clip_rect;
		QRectF					_trend_vector_clip_rect;
		QPainterPath			_inner_map_clip;
		QPainterPath			_outer_map_clip;
		QPen					_ndb_pen;
		QPen					_vor_pen;
		QPen					_dme_pen;
		QPen					_fix_pen;
		QPen					_home_pen;
		QPen					_lo_loc_pen;
		QPen					_hi_loc_pen;
		QFont					_radials_font;
		QPainterPath			_ndb_shape;
		QPolygonF				_dme_for_vor_shape;
		QPolygonF				_vor_shape;
		QPolygonF				_vortac_shape;
		QPolygonF				_home_shape;
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
		LocalParameters			_locals;
		LocalParameters			_locals_next;
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
	 * Set HSI parameters.
	 */
	void
	set_params (Parameters const&);

  private:
	// API of QWidget
	void
	resizeEvent (QResizeEvent*) override;

	// InstrumentWidget API
	void
	push_params() override;

  private:
	PaintWorkUnit	_local_paint_work_unit;
	Parameters		_params;
	LocalParameters	_locals;
};


inline void
HSIWidget::PaintWorkUnit::set_navaid_storage (NavaidStorage* navaid_storage)
{
	_navaid_storage = navaid_storage;
}


inline QPointF
HSIWidget::PaintWorkUnit::get_navaid_xy (LonLat const& position)
{
	if (!_params.position)
		return QPointF();
	QPointF navaid_pos = EARTH_MEAN_RADIUS.nm() * position.rotated (*_params.position).project_flat();
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
	_local_paint_work_unit.set_navaid_storage (navaid_storage);
	request_repaint();
}

#endif

