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
#include <QtGui/QPaintEvent>
#include <QtGui/QColor>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument_widget.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/navaid_storage.h>
#include <xefis/core/work_performer.h>
#include <xefis/utility/text_painter.h>
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
	typedef std::map<QString, Angle> HeadingBugs;

	constexpr static int UpdateEvent			= QEvent::User + 0;
	constexpr static int RequestRepaintEvent	= QEvent::User + 1;

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
		Knots					ground_speed				= 0.f;
		bool					ground_speed_visible		= false;
		Knots					true_air_speed				= 0.f;
		bool					true_air_speed_visible		= false;
		float					mach						= 0.f;
		bool					mach_visible				= false;
		Angle					track_deviation				= 0_deg;
		bool					trend_vector_visible		= false;
		Length					trend_vector_lookahead		= 5_nm;
		Length					altitude_reach_distance		= 0_nm;
		bool					altitude_reach_visible		= false;
		Angle					wind_from_magnetic_heading	= 0_deg;
		Knots					wind_tas_speed				= 0.f;
		bool					wind_information_visible	= false;
		LonLat					position					= { 0_deg, 0_deg };
		bool					navaids_visible				= false;
		bool					vor_visible					= false;
		bool					dme_visible					= false;
		bool					ndb_visible					= false;
		bool					loc_visible					= false;
		bool					fix_visible					= false;
		QString					highlighted_loc;
		QString					positioning_hint;
		bool					positioning_hint_visible	= false;
	};

	class PaintWorkUnit:
		public Xefis::WorkPerformer::Unit,
		public Xefis::InstrumentAids
	{
	  public:
		PaintWorkUnit (HSIWidget*, QSize size);

		~PaintWorkUnit() noexcept { }

		void
		set_params (Parameters const& params);

		void
		execute() override;

	  private:
		void
		resize (QSize size);

		void
		recalculate();

		void
		paint (QPainter&);

		void
		paint_aircraft (QPainter&, TextPainter&);

		void
		paint_hints (QPainter&, TextPainter&);

		void
		paint_ap_settings (QPainter&, TextPainter&);

		void
		paint_directions (QPainter&, TextPainter&);

		void
		paint_track (QPainter&, TextPainter&, bool paint_heading_triangle);

		void
		paint_altitude_reach (QPainter&);

		void
		paint_trend_vector (QPainter&, TextPainter&);

		void
		paint_speeds_and_wind (QPainter&, TextPainter&);

		void
		paint_range (QPainter&, TextPainter&);

		void
		paint_navaids (QPainter&, TextPainter&);

		void
		paint_locs (QPainter&, TextPainter&);

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

	  private:
		QImage					_image;
		HSIWidget*				_hsi;
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
	 * Set ground speed.
	 */
	void
	set_ground_speed (Knots);

	/**
	 * Toggle visibility of the ground speed.
	 */
	void
	set_ground_speed_visible (bool visible);

	/**
	 * Set true air speed.
	 */
	void
	set_true_air_speed (Knots);

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
	 * Set track estimation in degrees per mile flown.
	 * Positive degrees means turning to the right, negative - to the left.
	 */
	void
	set_track_deviation (Angle degrees_per_mile);

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
	set_wind_information (Angle wind_from_magnetic_heading, Knots wind_tas_speed);

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
	 * Set positioning hint.
	 */
	void
	set_positioning_hint (QString const& hint);

	/**
	 * Set positioning hint visibility.
	 */
	void
	set_positioning_hint_visible (bool visible);

  private:
	void
	update_later();

	void
	request_repaint();

	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

	void
	customEvent (QEvent*) override;

  private:
	NavaidStorage*			_navaid_storage				= nullptr;
	Xefis::WorkPerformer*	_work_performer				= nullptr;
	RecursiveMutex			_repaint_mutex;
	bool					_recalculation_needed		= false;
	bool					_repaint_requested			= false;
	bool					_worker_added				= false;
	bool					_queue_repaint				= false;
	QImage					_paint_buffer;
	PaintWorkUnit			_paint_work_unit;
	QSize					_safe_size;
	Parameters				_params;
};


inline
HSIWidget::PaintWorkUnit::PaintWorkUnit (HSIWidget* hsi_widget, QSize size):
	InstrumentAids (0.5f, 1.1f, 1.f),
	_image (size, QImage::Format_ARGB32_Premultiplied),
	_hsi (hsi_widget)
{ }


inline void
HSIWidget::PaintWorkUnit::set_params (Parameters const& params)
{
	_hsi->_repaint_mutex.synchronize ([&]() {
		_params_next = params;
	});
}


inline void
HSIWidget::PaintWorkUnit::resize (QSize size)
{
	InstrumentAids::update_sizes (size, _hsi->window()->size());
	_image = QImage (size, QImage::Format_ARGB32_Premultiplied);
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


// TODO set this in constructor only
inline void
HSIWidget::set_navaid_storage (NavaidStorage* navaid_storage)
{
	_navaid_storage = navaid_storage;
	request_repaint();
}


inline void
HSIWidget::set_display_mode (DisplayMode display_mode)
{
	_params.display_mode = display_mode;
	_recalculation_needed = true;
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
HSIWidget::set_ground_speed (Knots ground_speed)
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
HSIWidget::set_true_air_speed (Knots true_air_speed)
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
HSIWidget::set_track_deviation (Angle degrees_per_mile)
{
	_params.track_deviation = degrees_per_mile;
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
HSIWidget::set_wind_information (Angle wind_from_magnetic_heading, Knots wind_tas_speed)
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
HSIWidget::set_positioning_hint (QString const& hint)
{
	_params.positioning_hint = hint;
	request_repaint();
}


inline void
HSIWidget::set_positioning_hint_visible (bool visible)
{
	_params.positioning_hint_visible = visible;
}

#endif

