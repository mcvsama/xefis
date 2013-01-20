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

#ifndef XEFIS__WIDGETS__HSI_WIDGET_H__INCLUDED
#define XEFIS__WIDGETS__HSI_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QPaintEvent>
#include <QtGui/QColor>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument_widget.h>
#include <xefis/core/navaid_storage.h>
#include <xefis/utility/text_painter.h>
#include <xefis/utility/latlng.h>
#include <xefis/utility/numeric.h>


class HSIWidget: public Xefis::InstrumentWidget
{
	typedef std::map<QString, Degrees> HeadingBugs;

  public:
	// Ctor
	HSIWidget (QWidget* parent);

	/**
	 * Set reference to the nav storage.
	 * Object must be live as long as HSIWidget is live.
	 * Pass nullptr to deassign.
	 */
	void
	set_navaid_storage (NavaidStorage*);

	/**
	 * Return navigation range.
	 */
	Miles
	range() const;

	/**
	 * Set navigation range.
	 */
	void
	set_range (Miles miles);

	/**
	 * Return current ture heading value.
	 */
	Degrees
	true_heading() const;

	/**
	 * Set true heading value.
	 * Note that both magnetic and true heading must be set
	 * correctly for HSI to display navaids and scales properly.
	 */
	void
	set_true_heading (Degrees);

	/**
	 * Return current magnetic heading value.
	 */
	Degrees
	magnetic_heading() const;

	/**
	 * Set magnetic heading value.
	 * Note that both magnetic and true heading must be set
	 * correctly for HSI to display navaids and scales properly.
	 */
	void
	set_magnetic_heading (Degrees);

	/**
	 * Toggle heading scale visibility.
	 */
	void
	set_heading_visible (bool visible);

	/**
	 * Return A/P magnetic heading.
	 */
	Degrees
	ap_magnetic_heading() const;

	/**
	 * Set A/P magnetic heading.
	 */
	void
	set_ap_magnetic_heading (Degrees);

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
	 * Flight path heading (track).
	 */
	Degrees
	track() const;

	/**
	 * Set track magnetic heading.
	 */
	void
	set_track (Degrees);

	/**
	 * Set visibility of the track line.
	 */
	void
	set_track_visible (bool visible);

	/**
	 * Return current ground speed.
	 */
	Knots
	ground_speed() const;

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
	 * Return current true air speed.
	 */
	Knots
	true_air_speed() const;

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
	 * Return mach number.
	 */
	float
	mach() const;

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
	set_position (LatLng const&);

	/**
	 * Set track estimation in degrees per mile flown.
	 * Positive degrees means turning to the right, negative - to the left.
	 */
	void
	set_track_deviation (Degrees degrees_per_mile);

	/**
	 * Set track estimation visibility.
	 */
	void
	set_trend_vector_visible (bool visible);

	/**
	 * Set track estimation lookahead in nautical miles.
	 */
	void
	set_trend_vector_lookahead (Miles lookahead);

	/**
	 * Set dotted Earth visibility.
	 */
	void
	set_dotted_earth_visible (bool visible);

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

  private:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

	void
	paint_aircraft (QPainter&, TextPainter&, float q, float r);

	void
	paint_ap_settings (QPainter&, TextPainter&, float q, float r);

	void
	paint_directions (QPainter&, TextPainter&, float q, float r);

	void
	paint_track (QPainter&, TextPainter&, float q, float r);

	void
	paint_trend_vector (QPainter&, TextPainter&, float q, float r);

	void
	paint_speeds (QPainter&, TextPainter&, float q, float r);

	void
	paint_dotted_earth (QPainter&, float q, float r);

	void
	paint_navaids (QPainter&, TextPainter&, float q, float r);

	void
	paint_locs (QPainter&, TextPainter&, float q);

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
	get_navaid_xy (LatLng const& position);

	float
	nm_to_px (Miles miles);

  private:
	NavaidStorage*			_navaid_storage				= nullptr;

	// Cache:
	TextPainter::Cache		_text_painter_cache;
	QTransform				_aircraft_center_transform;
	QTransform				_mag_heading_transform;
	QTransform				_true_heading_transform;
	QRectF					_map_clip_rect;
	QRectF					_inside_map_clip_rect;
	QPainterPath			_inner_map_clip;
	QPainterPath			_outer_map_clip;
	QPen					_ndb_pen;
	QPen					_vor_pen;
	QPen					_dme_pen;
	QPen					_fix_pen;
	QPen					_lo_loc_pen;
	QPen					_hi_loc_pen;
	QPolygonF				_vor_shape;
	NavaidStorage::Navaids	_loc_navs;
	NavaidStorage::Navaids	_ndb_navs;
	NavaidStorage::Navaids	_vor_navs;
	NavaidStorage::Navaids	_dme_navs;
	NavaidStorage::Navaids	_fix_navs;
	QPolygonF				_aircraft_shape;
	QPolygonF				_ap_bug_shape;
	Degrees					_limited_rotation;

	// Parameters:
	Miles					_range						= 1.f;
	Degrees					_mag_heading				= 0.f;
	Degrees					_true_heading				= 0.f;
	bool					_heading_visible			= false;
	Degrees					_ap_mag_heading				= 0.f;
	bool					_ap_heading_visible			= false;
	bool					_ap_track_visible			= false;
	Degrees					_track_deg					= 0.f;
	bool					_track_visible				= false;
	Knots					_ground_speed				= 0.f;
	bool					_ground_speed_visible		= false;
	Knots					_true_air_speed				= 0.f;
	bool					_true_air_speed_visible		= false;
	float					_mach						= 0.f;
	bool					_mach_visible				= false;
	Degrees					_track_deviation			= 0.f;
	bool					_trend_vector_visible		= false;
	Miles					_trend_vector_lookahead		= 5.f;
	LatLng					_position					= { 0.f, 0.f };
	bool					_dotted_earth_visible		= false;
	bool					_navaids_visible			= false;
	bool					_vor_visible				= false;
	bool					_dme_visible				= false;
	bool					_ndb_visible				= false;
	bool					_loc_visible				= false;
	bool					_fix_visible				= false;
	QString					_highlighted_loc;
};


inline void
HSIWidget::set_navaid_storage (NavaidStorage* navaid_storage)
{
	_navaid_storage = navaid_storage;
	update();
}


inline Miles
HSIWidget::range() const
{
	return _range;
}


inline void
HSIWidget::set_range (Miles miles)
{
	_range = miles;
	update();
}


inline Degrees
HSIWidget::true_heading() const
{
	return _true_heading;
}


inline void
HSIWidget::set_true_heading (Degrees degrees)
{
	_true_heading = degrees;
	update();
}


inline Degrees
HSIWidget::magnetic_heading() const
{
	return _mag_heading;
}


inline void
HSIWidget::set_magnetic_heading (Degrees degrees)
{
	_mag_heading = degrees;
	update();
}


inline void
HSIWidget::set_heading_visible (bool visible)
{
	_heading_visible = visible;
	update();
}


inline Degrees
HSIWidget::ap_magnetic_heading() const
{
	return _ap_mag_heading;
}


inline void
HSIWidget::set_ap_magnetic_heading (Degrees heading)
{
	_ap_mag_heading = heading;
	update();
}


inline void
HSIWidget::set_ap_heading_visible (bool visible)
{
	_ap_heading_visible = visible;
	update();
}


inline void
HSIWidget::set_ap_track_visible (bool visible)
{
	_ap_track_visible = visible;
	update();
}


inline Degrees
HSIWidget::track() const
{
	return _track_deg;
}


inline void
HSIWidget::set_track (Degrees heading)
{
	_track_deg = heading;
	update();
}


inline void
HSIWidget::set_track_visible (bool visible)
{
	_track_visible = visible;
	update();
}


inline Knots
HSIWidget::ground_speed() const
{
	return _ground_speed;
}


inline void
HSIWidget::set_ground_speed (Knots ground_speed)
{
	_ground_speed = ground_speed;
	update();
}


inline void
HSIWidget::set_ground_speed_visible (bool visible)
{
	_ground_speed_visible = visible;
	update();
}


inline Knots
HSIWidget::true_air_speed() const
{
	return _true_air_speed;
}


inline void
HSIWidget::set_true_air_speed (Knots true_air_speed)
{
	_true_air_speed = true_air_speed;
	update();
}


inline void
HSIWidget::set_true_air_speed_visible (bool visible)
{
	_true_air_speed_visible = visible;
	update();
}


inline float
HSIWidget::mach() const
{
	return _mach;
}


inline void
HSIWidget::set_mach (float value)
{
	_mach = value;
	update();
}


inline void
HSIWidget::set_mach_visible (bool visible)
{
	_mach_visible = visible;
	update();
}


inline void
HSIWidget::set_position (LatLng const& position)
{
	_position = position;
	update();
}


inline void
HSIWidget::set_track_deviation (Degrees degrees_per_mile)
{
	_track_deviation = degrees_per_mile;
	update();
}


inline void
HSIWidget::set_trend_vector_visible (bool visible)
{
	_trend_vector_visible = visible;
	update();
}


inline void
HSIWidget::set_trend_vector_lookahead (Miles lookahead)
{
	_trend_vector_lookahead = lookahead;
	update();
}


inline void
HSIWidget::set_dotted_earth_visible (bool visible)
{
	_dotted_earth_visible = visible;
	update();
}


inline void
HSIWidget::set_navaids_visible (bool visible)
{
	_navaids_visible = visible;
	update();
}


inline void
HSIWidget::set_vor_visible (bool visible)
{
	_vor_visible = visible;
	update();
}


inline void
HSIWidget::set_dme_visible (bool visible)
{
	_dme_visible = visible;
	update();
}


inline void
HSIWidget::set_ndb_visible (bool visible)
{
	_ndb_visible = visible;
	update();
}


inline void
HSIWidget::set_loc_visible (bool visible)
{
	_loc_visible = visible;
	update();
}


inline void
HSIWidget::set_fix_visible (bool visible)
{
	_fix_visible = visible;
	update();
}


inline void
HSIWidget::set_highlighted_loc (QString const& identifier)
{
	_highlighted_loc = identifier;
}


inline QPointF
HSIWidget::get_navaid_xy (LatLng const& position)
{
	QPointF navaid_pos = EARTH_MEAN_RADIUS_NM * position.rotated (_position).project_flat();
	return _true_heading_transform.map (QPointF (nm_to_px (navaid_pos.x()), nm_to_px (navaid_pos.y())));
}


inline float
HSIWidget::nm_to_px (Miles miles)
{
	return 0.55f * wh() * miles / _range;
}

#endif

