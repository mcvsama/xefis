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

#ifndef XEFIS__WIDGETS__EFIS_NAV_WIDGET_H__INCLUDED
#define XEFIS__WIDGETS__EFIS_NAV_WIDGET_H__INCLUDED

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


class EFISNavWidget: public Xefis::InstrumentWidget
{
	typedef std::map<QString, Degrees> HeadingBugs;

  public:
	// Ctor
	EFISNavWidget (QWidget* parent);

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
	set_heading_visibility (bool visible);

	/**
	 * Flight path heading (track).
	 */
	Degrees
	flight_path_beta() const;

	/**
	 * Set flight path heading.
	 */
	void
	set_flight_path_beta (Degrees);

	/**
	 * Set visibility of the flight path.
	 */
	void
	set_flight_path_marker_visibility (bool visible);

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
	set_ground_speed_visibility (bool visible);

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
	set_true_air_speed_visibility (bool visible);

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
	set_mach_visibility (bool visible);

  protected:
	void
	paintEvent (QPaintEvent*) override;

  private:
	QTransform			_aircraft_center_transform;
	TextPainter::Cache	_text_painter_cache;

	// Parameters:
	Degrees				_heading					= 0.f;
	bool				_heading_visibility			= false;
	Degrees				_flight_path_beta			= 0.f;
	bool				_flight_path_visible		= false;
	Knots				_ground_speed				= 0.f;
	bool				_ground_speed_visible		= false;
	Knots				_true_air_speed				= 0.f;
	bool				_true_air_speed_visible		= false;
	float				_mach						= 0.f;
	bool				_mach_visible				= false;
};


inline Degrees
EFISNavWidget::heading() const
{
	return _heading;
}


inline void
EFISNavWidget::set_heading (Degrees degrees)
{
	_heading = degrees;
	update();
}


inline void
EFISNavWidget::set_heading_visibility (bool visible)
{
	_heading_visibility = visible;
	update();
}


inline Degrees
EFISNavWidget::flight_path_beta() const
{
	return _flight_path_beta;
}


inline void
EFISNavWidget::set_flight_path_beta (Degrees heading)
{
	_flight_path_beta = heading;
	update();
}


inline void
EFISNavWidget::set_flight_path_marker_visibility (bool visible)
{
	_flight_path_visible = visible;
	update();
}


inline Knots
EFISNavWidget::ground_speed() const
{
	return _ground_speed;
}


inline void
EFISNavWidget::set_ground_speed (Knots ground_speed)
{
	_ground_speed = ground_speed;
	update();
}


inline void
EFISNavWidget::set_ground_speed_visibility (bool visible)
{
	_ground_speed_visible = visible;
	update();
}


inline Knots
EFISNavWidget::true_air_speed() const
{
	return _true_air_speed;
}


inline void
EFISNavWidget::set_true_air_speed (Knots true_air_speed)
{
	_true_air_speed = true_air_speed;
	update();
}


inline void
EFISNavWidget::set_true_air_speed_visibility (bool visible)
{
	_true_air_speed_visible = visible;
	update();
}


inline float
EFISNavWidget::mach() const
{
	return _mach;
}


inline void
EFISNavWidget::set_mach (float value)
{
	_mach = value;
	update();
}


inline void
EFISNavWidget::set_mach_visibility (bool visible)
{
	_mach_visible = visible;
	update();
}

#endif

