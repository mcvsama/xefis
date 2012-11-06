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

#ifndef XEFIS__INSTRUMENTS__EFIS_H__INCLUDED
#define XEFIS__INSTRUMENTS__EFIS_H__INCLUDED

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QPaintEvent>
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <QtNetwork/QUdpSocket>

// Standard:
#include <cstddef>
#include <map>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/text_painter.h>


class EFIS: public QWidget
{
	Q_OBJECT

	typedef std::map<QString, Knots> SpeedBugs;
	typedef std::map<QString, Feet> AltitudeBugs;

  public:
	static const char* AP;

  public:
	EFIS (QWidget* parent);

	/**
	 * Show input alert when data is not coming for given period of time.
	 * Pass 0.f to disable. Default is 110 ms.
	 */
	void
	set_input_alert_timeout (Seconds timeout);

	Degrees
	roll() const;

	void
	set_roll (Degrees);

	Degrees
	pitch() const;

	void
	set_pitch (Degrees);

	Degrees
	heading() const;

	void
	set_heading (Degrees);

	Knots
	ias() const;

	void
	set_ias (Knots);

	Feet
	altitude() const;

	void
	set_altitude (Feet);

	Feet
	climb_rate() const;

	void
	set_climb_rate (Feet feet_per_minute);

	/**
	 * Return speed bug value or 0.0f if not found.
	 */
	Knots
	speed_bug (QString name) const;

	/**
	 * Add new speed bug. A special value EFIS::AP ("A/P")
	 * renders autopilot-style bug instead of a regular one.
	 */
	void
	add_speed_bug (QString name, Knots speed);

	/**
	 * Remove a speed bug.
	 */
	void
	remove_speed_bug (QString name);

	/**
	 * Return altitude bug value of 0.0f if not found.
	 */
	Feet
	altitude_bug (QString name) const;

	/**
	 * Add new altitude bug. A special value EFIS::AP ("A/P")
	 * renders autopilot-style bug instead of a regular one.
	 */
	void
	add_altitude_bug (QString name, Feet altitude);

	/**
	 * Remove an altitude bug.
	 */
	void
	remove_altitude_bug (QString name);

	/**
	 * Return field of view.
	 * Default is 120°. Usable maximum: 180°.
	 */
	Degrees
	fov() const;

	/**
	 * Set field of view.
	 */
	void
	set_fov (Degrees);

  public slots:
	/**
	 * Read and apply FlightGear datagrams from UDP socket.
	 */
	void
	read_input();

	/**
	 * Show input alert (when there's no incoming
	 * data from external source).
	 */
	void
	input_timeout();

	/**
	 * Hide input alert.
	 */
	void
	input_ok();

  protected:
	void
	paintEvent (QPaintEvent*) override;

	void
	resizeEvent (QResizeEvent*) override;

	void
	paint_horizon (QPainter&);

	void
	paint_pitch_scale (QPainter&);

	void
	paint_heading (QPainter&);

	void
	paint_roll (QPainter&);

	void
	paint_center_cross (QPainter&);

	void
	paint_speed (QPainter&);

	void
	paint_altitude (QPainter&);

	void
	paint_climb_rate (QPainter&);

	void
	paint_input_alert (QPainter&);

  private:
	QPainterPath
	get_pitch_scale_clipping_path() const;

	float
	pitch_to_px (Degrees degrees) const;

	float
	heading_to_px (Degrees degrees) const;

	float
	scale_cbr (Feet climb_rate) const;

	float
	pen_width (float scale = 1.0f) const;

	float
	font_size (float scale = 1.0f) const;

	int
	get_digit_width (QFont&) const;

	void
	update_fonts();

  private:
	QFont				_font_10_bold;
	QFont				_font_13_bold;
	QFont				_font_16_bold;
	QFont				_font_20_bold;
	float				_font_10_digit_width;
	float				_font_13_digit_width;
	float				_font_16_digit_width;
	float				_font_20_digit_width;
	float				_font_10_digit_height;
	float				_font_13_digit_height;
	float				_font_16_digit_height;
	float				_font_20_digit_height;
	QColor				_sky_color;
	QColor				_ground_color;
	QColor				_ladder_color;
	QColor				_autopilot_color;
	QTransform			_center_transform;
	QTransform			_pitch_transform;
	QTransform			_roll_transform;
	QTransform			_heading_transform;
	QTransform			_horizon_transform;
	QFont				_font;
	Degrees				_fov						= 120.f;
	QUdpSocket*			_input						= nullptr;
	Seconds				_input_alert_timeout		= 0.0f;
	QTimer*				_input_alert_timer			= nullptr;
	QTimer*				_input_alert_hide_timer		= nullptr;
	bool				_show_input_alert			= false;
	TextPainter::Cache	_text_painter_cache;

	// Parameters:
	Degrees				_pitch						= 0.f;
	Degrees				_roll						= 0.f;
	Degrees				_heading					= 0.f;
	Knots				_ias						= 0.f;
	Feet				_altitude					= 0.f;
	Feet				_cbr						= 0.f;
	SpeedBugs			_speed_bugs;
	AltitudeBugs		_altitude_bugs;

	static const char	DIGITS[];
	static const char*	MINUS_SIGN;
};


inline Degrees
EFIS::roll() const
{
	return _roll;
}


inline void
EFIS::set_roll (Degrees degrees)
{
	_roll = degrees;
	update();
}


inline Degrees
EFIS::pitch() const
{
	return _pitch;
}


inline void
EFIS::set_pitch (Degrees degrees)
{
	_pitch = degrees;
	update();
}


inline Degrees
EFIS::heading() const
{
	return _heading;
}


inline void
EFIS::set_heading (Degrees degrees)
{
	_heading = degrees;
	update();
}


inline Knots
EFIS::ias() const
{
	return _ias;
}


inline void
EFIS::set_ias (Knots ias)
{
	_ias = ias;
	update();
}


inline Feet
EFIS::altitude() const
{
	return _altitude;
}


inline void
EFIS::set_altitude (Feet altitude)
{
	_altitude = altitude;
	update();
}


inline Feet
EFIS::climb_rate() const
{
	return _cbr;
}


inline void
EFIS::set_climb_rate (Feet feet_per_minute)
{
	_cbr = feet_per_minute;
	update();
}


inline Knots
EFIS::speed_bug (QString name) const
{
	auto it = _speed_bugs.find (name);
	if (it != _speed_bugs.end())
		return it->second;
	return 0.f;
}


inline void
EFIS::add_speed_bug (QString name, Knots speed)
{
	_speed_bugs[name] = speed;
	update();
}


inline void
EFIS::remove_speed_bug (QString name)
{
	_speed_bugs.erase (name);
	update();
}


inline Feet
EFIS::altitude_bug (QString name) const
{
	auto it = _altitude_bugs.find (name);
	if (it != _altitude_bugs.end())
		return it->second;
	return 0.f;
}


inline void
EFIS::add_altitude_bug (QString name, Feet altitude)
{
	_altitude_bugs[name] = altitude;
	update();
}


inline void
EFIS::remove_altitude_bug (QString name)
{
	_altitude_bugs.erase (name);
	update();
}


inline Degrees
EFIS::fov() const
{
	return _fov;
}


inline void
EFIS::set_fov (Degrees degrees)
{
	_fov = degrees;
	update();
}


inline float
EFIS::pitch_to_px (Degrees degrees) const
{
	float const correction = 0.775f;
	return -degrees / (_fov * correction) * std::min (width(), height());
}


inline float
EFIS::heading_to_px (Degrees degrees) const
{
	return pitch_to_px (-degrees);
}

inline float
EFIS::pen_width (float scale) const
{
	return scale * std::min (width(), height()) / 325.f;
}


inline float
EFIS::font_size (float scale) const
{
	return scale * std::min (width(), height()) / 375.f;
}

#endif

