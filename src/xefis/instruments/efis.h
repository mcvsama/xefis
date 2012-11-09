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

	class AltitudeLadder
	{
	  public:
		AltitudeLadder (EFIS&, QPainter&);

		void
		paint();

	  private:
		void
		paint_black_box (float x, bool only_compute_black_box_rect = false);

		void
		paint_ladder_scale (float x);

		void
		paint_bugs (float x);

		void
		paint_climb_rate (float x);

		void
		paint_pressure (float x);

		void
		paint_ap_setting (float x);

		float
		ft_to_px (Feet ft) const;

		float
		scale_cbr (Feet climb_rate) const;

	  private:
		EFIS&			_efis;
		QPainter&		_painter;
		TextPainter		_text_painter;
		Feet			_altitude;
		FeetPerMinute	_climb_rate;
		float			_pressure;
		Feet			_extent;
		float			_sgn;
		Feet			_min_shown;
		Feet			_max_shown;
		int				_rounded_altitude;
		QRectF			_ladder_rect;
		QPen			_ladder_pen;
		QRectF			_black_box_rect;
		QPen			_black_box_pen;
		QPen			_scale_pen_1;
		QPen			_scale_pen_2; // Bold one, each 500 ft
		QPen			_negative_altitude_pen;
		QPen			_altitude_bug_pen;
		QPen			_ldg_alt_pen;
	};

	class SpeedLadder
	{
	  public:
		SpeedLadder (EFIS&, QPainter&);

		void
		paint();

	  private:
		void
		paint_black_box (float x, bool only_compute_black_box_rect = false);

		void
		paint_ladder_scale (float x);

		void
		paint_speed_limits (float x);

		void
		paint_bugs (float x);

		void
		paint_ap_setting (float x);

		float
		kt_to_px (Knots ft) const;

	  private:
		EFIS&		_efis;
		QPainter&	_painter;
		TextPainter	_text_painter;
		Knots		_speed;
		Knots		_minimum_speed;
		Knots		_warning_speed;
		Knots		_maximum_speed;
		Knots		_extent;
		Knots		_min_shown;
		Knots		_max_shown;
		int			_rounded_speed;
		QRectF		_ladder_rect;
		QPen		_ladder_pen;
		QRectF		_black_box_rect;
		QPen		_black_box_pen;
		QPen		_scale_pen;
		QPen		_speed_bug_pen;
	};

	class AttitudeDirectorIndicator
	{
	  public:
		AttitudeDirectorIndicator (EFIS&, QPainter&);

		void
		paint();

	  private:
		void
		paint_horizon();

		void
		paint_pitch();

		void
		paint_roll();

		void
		paint_heading();

		QPainterPath
		get_pitch_scale_clipping_path() const;

		float
		pitch_to_px (Degrees degrees) const;

		float
		heading_to_px (Degrees degrees) const;

	  private:
		EFIS&		_efis;
		QPainter&	_painter;
		TextPainter	_text_painter;
		Degrees		_pitch;
		Degrees		_roll;
		Degrees		_heading;
		QTransform	_pitch_transform;
		QTransform	_roll_transform;
		QTransform	_heading_transform;
		QTransform	_horizon_transform;
	};

  public:
	// Autopilot bug:
	static const char* AP;
	// Landing altitude bug:
	static const char* LDGALT;
	// Autothrottle bug:
	static const char* AT;

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
	speed() const;

	void
	set_speed (Knots);

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
	 * Add new speed bug. A special value EFIS::AT ("A/T")
	 * renders autothrottle-style bug instead of a regular one.
	 */
	void
	add_speed_bug (QString name, Knots speed);

	/**
	 * Remove a speed bug.
	 * Pass QString::null to remove all speed bugs.
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
	 * Pass QString::null to remove all altitude bugs.
	 */
	void
	remove_altitude_bug (QString name);

	/**
	 * Return current pressure indicator value.
	 */
	InHg
	pressure() const;

	/**
	 * Set pressure indicator (inHg)
	 */
	void
	set_pressure (InHg pressure);

	/**
	 * Show or hide pressure indicator.
	 */
	void
	set_pressure_visibility (bool visible);

	/**
	 * Get minimum speed indicator setting.
	 */
	Knots
	minimum_speed() const;

	/**
	 * Set minimum speed indicator on the speed ladder.
	 */
	void
	set_minimum_speed (Knots);

	/**
	 * Set minimum speed indicator visibility.
	 */
	void
	set_minimum_speed_visibility (bool visible);

	/**
	 * Get warning speed indicator setting.
	 */
	Knots
	warning_speed() const;

	/**
	 * Set warning speed indicator on the speed ladder.
	 */
	void
	set_warning_speed (Knots);

	/**
	 * Set warning speed indicator visibility.
	 */
	void
	set_warning_speed_visibility (bool visible);

	/**
	 * Get maximum speed indicator setting.
	 */
	Knots
	maximum_speed() const;

	/**
	 * Set maximum speed indicator on the speed ladder.
	 */
	void
	set_maximum_speed (Knots);

	/**
	 * Set maximum speed indicator visibility.
	 */
	void
	set_maximum_speed_visibility (bool visible);

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
	paint_center_cross (QPainter&);

	void
	paint_input_alert (QPainter&);

  private:
	/**
	 * Return min (width(), height());
	 */
	float
	wh() const;

	QPen
	get_pen (QColor const& color, float width);

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
	QColor				_ladder_border_color;
	QColor				_autopilot_color;
	QColor				_navigation_color;
	QTransform			_center_transform;
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
	Knots				_speed						= 0.f;
	Feet				_altitude					= 0.f;
	Feet				_climb_rate					= 0.f;
	SpeedBugs			_speed_bugs;
	AltitudeBugs		_altitude_bugs;
	InHg				_pressure					= 0.f;
	bool				_pressure_visible			= false;
	Knots				_minimum_speed				= 0.f;
	bool				_minimum_speed_visible		= false;
	Knots				_warning_speed				= 0.f;
	bool				_warning_speed_visible		= false;
	Knots				_maximum_speed				= 0.f;
	bool				_maximum_speed_visible		= false;

	static const char	DIGITS[];
	static const char*	MINUS_SIGN;
};


inline float
EFIS::AltitudeLadder::ft_to_px (Feet ft) const
{
	return -0.5f * _ladder_rect.height() * (ft - _altitude) / (_extent / 2.f);
}


inline float
EFIS::SpeedLadder::kt_to_px (Knots kt) const
{
	return -0.5f * _ladder_rect.height() * (kt - _speed) / (_extent / 2.f);
}


inline float
EFIS::AttitudeDirectorIndicator::pitch_to_px (Degrees degrees) const
{
	float const correction = 0.775f;
	return -degrees / (_efis._fov * correction) * _efis.wh();
}


inline float
EFIS::AttitudeDirectorIndicator::heading_to_px (Degrees degrees) const
{
	return pitch_to_px (-degrees);
}


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
EFIS::speed() const
{
	return _speed;
}


inline void
EFIS::set_speed (Knots speed)
{
	_speed = speed;
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
	return _climb_rate;
}


inline void
EFIS::set_climb_rate (Feet feet_per_minute)
{
	_climb_rate = feet_per_minute;
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
	if (name.isNull())
		_speed_bugs.clear();
	else
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
	if (name.isNull())
		_altitude_bugs.clear();
	else
		_altitude_bugs.erase (name);
	update();
}


inline void
EFIS::set_pressure (InHg pressure)
{
	_pressure = pressure;
	update();
}


inline InHg
EFIS::pressure() const
{
	return _pressure;
}


inline void
EFIS::set_pressure_visibility (bool visible)
{
	_pressure_visible = visible;
	update();
}


inline Knots
EFIS::minimum_speed() const
{
	return _minimum_speed;
}


inline void
EFIS::set_minimum_speed (Knots minimum_speed)
{
	_minimum_speed = minimum_speed;
	update();
}


inline void
EFIS::set_minimum_speed_visibility (bool visible)
{
	_minimum_speed_visible = visible;
	update();
}


inline Knots
EFIS::warning_speed() const
{
	return _warning_speed;
}


inline void
EFIS::set_warning_speed (Knots warning_speed)
{
	_warning_speed = warning_speed;
	update();
}


inline void
EFIS::set_warning_speed_visibility (bool visible)
{
	_warning_speed_visible = visible;
	update();
}


inline Knots
EFIS::maximum_speed() const
{
	return _maximum_speed;
}


inline void
EFIS::set_maximum_speed (Knots maximum_speed)
{
	_maximum_speed = maximum_speed;
	update();
}


inline void
EFIS::set_maximum_speed_visibility (bool visible)
{
	_maximum_speed_visible = visible;
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
EFIS::wh() const
{
	return std::min (0.8f * width(), 1.f * height());
}


inline QPen
EFIS::get_pen (QColor const& color, float width)
{
	return QPen (color, pen_width (width), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}


inline float
EFIS::pen_width (float scale) const
{
	return scale * wh() / 325.f;
}


inline float
EFIS::font_size (float scale) const
{
	return scale * wh() / 375.f;
}

#endif

