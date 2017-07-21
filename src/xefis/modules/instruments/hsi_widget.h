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
#include <xefis/support/navigation/navaid_storage.h>
#include <xefis/support/system/work_performer.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/mutex.h>


using xf::NavaidStorage;

class HSIWidget: public xf::InstrumentWidget
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
		// TODO optionals must not have default values
		DisplayMode						display_mode				= DisplayMode::Expanded;
		HeadingMode						heading_mode				= HeadingMode::Magnetic;
		Length							range						= 1_nmi;
		std::optional<si::Angle>		heading_magnetic;
		std::optional<si::Angle>		heading_true;
		bool							ap_visible					= false;
		bool							ap_line_visible				= false;
		std::optional<Angle>			ap_heading_magnetic;
		std::optional<Angle>			ap_track_magnetic;
		std::optional<bool>				ap_use_trk;
		bool							track_visible				= false;
		std::optional<si::Angle>		track_magnetic;
		bool							course_visible				= false;
		std::optional<Angle>			course_setting_magnetic;
		std::optional<Angle>			course_deviation;
		std::optional<bool>				course_to_flag;
		QString							navaid_selected_reference;
		QString							navaid_selected_identifier;
		std::optional<Length>			navaid_selected_distance;
		std::optional<Time>				navaid_selected_eta;
		std::optional<Angle>			navaid_selected_course_magnetic;
		int								navaid_left_type			= 0;
		QString							navaid_left_reference;
		QString							navaid_left_identifier;
		std::optional<Length>			navaid_left_distance;
		std::optional<Angle>			navaid_left_initial_bearing_magnetic;
		int								navaid_right_type			= 0;
		QString							navaid_right_reference;
		QString							navaid_right_identifier;
		std::optional<Length>			navaid_right_distance;
		std::optional<Angle>			navaid_right_initial_bearing_magnetic;
		std::optional<Length>			navigation_required_performance;
		std::optional<Length>			navigation_actual_performance;
		bool							center_on_track				= false;
		bool							home_track_visible			= false;
		std::optional<Angle>			true_home_direction;
		std::optional<si::Length>		dist_to_home_ground;
		std::optional<si::Length>		dist_to_home_vlos;
		std::optional<si::Length>		dist_to_home_vert;
		std::optional<LonLat>			home;
		std::optional<Speed>			ground_speed;
		std::optional<Speed>			true_air_speed;
		std::optional<AngularVelocity>	track_lateral_rotation;
		std::optional<si::Length>		altitude_reach_distance;
		std::optional<si::Angle>		wind_from_magnetic_heading;
		std::optional<si::Velocity>		wind_tas_speed;
		std::optional<si::LonLat>		position;
		bool							navaids_visible				= false;
		bool							fix_visible					= false;
		bool							vor_visible					= false;
		bool							dme_visible					= false;
		bool							ndb_visible					= false;
		bool							loc_visible					= false;
		bool							arpt_visible				= false;
		QString							highlighted_loc;
		std::optional<QString>			positioning_hint;
		std::optional<bool>				tcas_on;
		std::optional<Length>			tcas_range;
		Length							arpt_runways_range_threshold;
		Length							arpt_map_range_threshold;
		Length							arpt_runway_extension_length;
		std::array<Time, 3>				trend_vector_times;
		std::array<Length, 3>			trend_vector_min_ranges;
		Length							trend_vector_max_range;
		bool							round_clip					= false;

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
		std::optional<si::Angle>	heading;					// Computed mag or true, depending on heading mode.
		std::optional<si::Angle>	ap_bug_magnetic;			// Computed mag or true, depending on heading mode.
		std::optional<bool>			ap_use_trk;
		std::optional<si::Angle>	course_heading;				// Computed mag or true, depending on heading mode.
		std::optional<si::Angle>	track_true;					// Computed.
		std::optional<si::Angle>	track;						// Mag or true, depending on heading mode.
		std::optional<si::Angle>	rotation;
		QDateTime					positioning_hint_ts			= QDateTime::fromTime_t (0);
		bool						navaid_selected_visible		= false;
		bool						navaid_left_visible			= false;
		bool						navaid_right_visible		= false;
	};

	class PaintWorkUnit:
		public InstrumentWidget::PaintWorkUnit,
		protected xf::InstrumentAids
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
		paint_aircraft (xf::Painter&);

		void
		paint_navperf (xf::Painter&);

		void
		paint_hints (xf::Painter&);

		void
		paint_ap_settings (xf::Painter&);

		void
		paint_directions (xf::Painter&);

		void
		paint_track (xf::Painter&, bool paint_heading_triangle);

		void
		paint_altitude_reach (xf::Painter&);

		void
		paint_trend_vector (xf::Painter&);

		void
		paint_speeds_and_wind (xf::Painter&);

		void
		paint_home_direction (xf::Painter&);

		void
		paint_course (xf::Painter&);

		void
		paint_selected_navaid_info();

		void
		paint_tcas_and_navaid_info();

		void
		paint_pointers (xf::Painter&);

		void
		paint_range (xf::Painter&);

		void
		paint_navaids (xf::Painter&);

		void
		paint_locs();

		void
		paint_tcas();

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
		get_navaid_xy (LonLat const& navaid_position);

		/**
		 * Trend vector range.
		 */
		Length
		actual_trend_range() const;

		/**
		 * Gap between lines on trend vector.
		 */
		Length
		trend_gap() const;

		/**
		 * Time gap between lines on trend vector.
		 */
		Time
		trend_time_gap() const;

		float
		to_px (Length miles);

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
		QColor					_cyan						= { 0, 180, 255 };
		QPen					_ndb_pen;
		QPen					_vor_pen;
		QPen					_dme_pen;
		QPen					_fix_pen;
		QPen					_arpt_pen;
		QPen					_home_pen;
		QPen					_lo_loc_pen;
		QPen					_hi_loc_pen;
		QFont					_radials_font;
		QPolygonF				_dme_for_vor_shape;
		QPolygonF				_vor_shape;
		QPolygonF				_vortac_shape;
		QPolygonF				_home_shape;
		QPolygonF				_aircraft_shape;
		QPolygonF				_ap_bug_shape;
		bool					_navs_retrieved				= false;
		LonLat					_navs_retrieve_position		= { 0_deg, 0_deg };
		Length					_navs_retrieve_range		= 0_nmi;
		NavaidStorage::Navaids	_fix_navs;
		NavaidStorage::Navaids	_vor_navs;
		NavaidStorage::Navaids	_dme_navs;
		NavaidStorage::Navaids	_ndb_navs;
		NavaidStorage::Navaids	_loc_navs;
		NavaidStorage::Navaids	_arpt_navs;
		Parameters				_params;
		Parameters				_params_next;
		LocalParameters			_locals;
		LocalParameters			_locals_next;
	};

  public:
	// Ctor
	HSIWidget (QWidget* parent, xf::WorkPerformer*);

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


inline void
HSIWidget::set_navaid_storage (NavaidStorage* navaid_storage)
{
	_local_paint_work_unit.set_navaid_storage (navaid_storage);
	request_repaint();
}

#endif

