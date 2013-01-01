/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <QtGui/QWidget>
#include <QtGui/QPaintEvent>
#include <QtGui/QColor>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument_widget.h>
#include <xefis/utility/text_painter.h>
#include <xefis/utility/latlng.h>


class HSIWidget: public Xefis::InstrumentWidget
{
	typedef std::map<QString, Degrees> HeadingBugs;

  public:
	// Ctor
	HSIWidget (QWidget* parent);

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
	 * Return current heading value.
	 */
	Degrees
	heading() const;

	/**
	 * Set heading value.
	 */
	void
	set_heading (Degrees);

	/**
	 * Toggle heading scale visibility.
	 */
	void
	set_heading_visible (bool visible);

	/**
	 * Return A/P heading.
	 */
	Degrees
	ap_heading() const;

	/**
	 * Set A/P heading.
	 */
	void
	set_ap_heading (Degrees);

	/**
	 * Set A/P heading visibility.
	 */
	void
	set_ap_heading_visible (bool visible);

	/**
	 * Flight path heading (track).
	 */
	Degrees
	track() const;

	/**
	 * Set track heading.
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
	 * Set track estimation in degrees per mile flown.
	 * Positive degrees means turning to the right, negative - to the left.
	 */
	void
	set_track_deviation (Degrees degrees_per_mile);

	/**
	 * Set track estimation visibility.
	 */
	void
	set_track_estimation_visible (bool visible);

	/**
	 * Set track estimation lookahead in nautical miles.
	 */
	void
	set_track_estimation_lookahead (Miles lookahead);

  protected:
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
	paint_track_estimation (QPainter&, TextPainter&, float q, float r);

	void
	paint_speeds (QPainter&, TextPainter&, float q, float r);

  private:
	float
	nm_to_px (Miles miles);

  private:
	QTransform			_aircraft_center_transform;
	QTransform			_heading_transform;
	QRectF				_map_clip_rect;
	QRectF				_inside_map_clip_rect;
	TextPainter::Cache	_text_painter_cache;

	// Parameters:
	Miles					_range						= 1.f;
	Degrees					_heading					= 0.f;
	bool					_heading_visible			= false;
	Degrees					_ap_heading					= 0.f;
	bool					_ap_heading_visible			= false;
	Degrees					_track_deg					= 0.f;
	bool					_track_visible				= false;
	Knots					_ground_speed				= 0.f;
	bool					_ground_speed_visible		= false;
	Knots					_true_air_speed				= 0.f;
	bool					_true_air_speed_visible		= false;
	float					_mach						= 0.f;
	bool					_mach_visible				= false;
	Degrees					_track_deviation			= 0.f;
	bool					_track_estimation_visible	= false;
	Miles					_track_estimation_lookahead	= 5.f;
};


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
HSIWidget::heading() const
{
	return _heading;
}


inline void
HSIWidget::set_heading (Degrees degrees)
{
	_heading = degrees;
	update();
}


inline void
HSIWidget::set_heading_visible (bool visible)
{
	_heading_visible = visible;
	update();
}


inline Degrees
HSIWidget::ap_heading() const
{
	return _ap_heading;
}


inline void
HSIWidget::set_ap_heading (Degrees heading)
{
	_ap_heading = heading;
	update();
}


inline void
HSIWidget::set_ap_heading_visible (bool visible)
{
	_ap_heading_visible = visible;
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
HSIWidget::set_track_deviation (Degrees degrees_per_mile)
{
	_track_deviation = degrees_per_mile;
	update();
}


inline void
HSIWidget::set_track_estimation_visible (bool visible)
{
	_track_estimation_visible = visible;
	update();
}


inline void
HSIWidget::set_track_estimation_lookahead (Miles lookahead)
{
	_track_estimation_lookahead = lookahead;
	update();
}


inline float
HSIWidget::nm_to_px (Miles miles)
{
	return 0.5f * wh() * miles / _range;
}

#endif

