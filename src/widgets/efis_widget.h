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

#ifndef XEFIS__WIDGETS__EFIS_WIDGET_H__INCLUDED
#define XEFIS__WIDGETS__EFIS_WIDGET_H__INCLUDED

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QPaintEvent>
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <QtGui/QPainterPath>
#include <QtNetwork/QUdpSocket>

// Standard:
#include <cstddef>
#include <map>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/text_painter.h>


class EFISWidget: public QWidget
{
	Q_OBJECT

	typedef std::map<QString, Knots> SpeedBugs;
	typedef std::map<QString, Feet> AltitudeBugs;

	class AltitudeLadder
	{
	  public:
		AltitudeLadder (EFISWidget&, QPainter&);

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
		scale_cbr (FeetPerMinute climb_rate) const;

	  private:
		EFISWidget&		_efis;
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
		SpeedLadder (EFISWidget&, QPainter&);

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
		paint_speed_tendency (float x);

		void
		paint_bugs (float x);

		void
		paint_mach_number (float x);

		void
		paint_ap_setting (float x);

		float
		kt_to_px (Knots ft) const;

	  private:
		EFISWidget&	_efis;
		QPainter&	_painter;
		TextPainter	_text_painter;
		Knots		_speed;
		float		_mach;
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
		AttitudeDirectorIndicator (EFISWidget&, QPainter&);

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

		void
		paint_flight_path_marker();

		QPainterPath
		get_pitch_scale_clipping_path() const;

		float
		pitch_to_px (Degrees degrees) const;

		float
		heading_to_px (Degrees degrees) const;

	  private:
		EFISWidget&		_efis;
		QPainter&		_painter;
		TextPainter		_text_painter;
		Degrees			_pitch;
		Degrees			_roll;
		Degrees			_heading;
		QTransform		_pitch_transform;
		QTransform		_roll_transform;
		QTransform		_heading_transform;
		QTransform		_horizon_transform;
		QPainterPath	_flight_path_marker;
	};

  public:
	// Autopilot bug:
	static const char* AP;
	// Landing altitude bug:
	static const char* LDGALT;
	// Autothrottle bug:
	static const char* AT;

  public:
	EFISWidget (QWidget* parent);

	/**
	 * Return current pitch value.
	 */
	Degrees
	pitch() const;

	/**
	 * Set pitch value.
	 */
	void
	set_pitch (Degrees);

	/**
	 * Toggle pitch scale visibility.
	 * Toggles also artifical horizon.
	 */
	void
	set_pitch_visibility (bool visible);

	/**
	 * Return current roll value.
	 */
	Degrees
	roll() const;

	/**
	 * Set roll value.
	 */
	void
	set_roll (Degrees);

	/**
	 * Toggle roll scale visibility.
	 * Toggles also artifical horizon.
	 */
	void
	set_roll_visibility (bool visible);

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
	 * Flight path pitch.
	 */
	Degrees
	flight_path_alpha() const;

	/**
	 * Set flight path pitch.
	 */
	void
	set_flight_path_alpha (Degrees);

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
	 * Set visibility of the Flight Path Marker.
	 */
	void
	set_flight_path_marker_visibility (bool visible);

	/**
	 * Return current speed.
	 */
	Knots
	speed() const;

	/**
	 * Set speed shown on speed ladder.
	 */
	void
	set_speed (Knots);

	/**
	 * Toggle visibility of the speed scale.
	 */
	void
	set_speed_visibility (bool visible);

	/**
	 * Current altitude value.
	 */
	Feet
	altitude() const;

	/**
	 * Set altitude value.
	 */
	void
	set_altitude (Feet);

	/**
	 * Toggle visibility of the altitude scale.
	 */
	void
	set_altitude_visibility (bool visible);

	/**
	 * Return current climb rate.
	 */
	FeetPerMinute
	climb_rate() const;

	/**
	 * Set climb rate.
	 */
	void
	set_climb_rate (FeetPerMinute feet_per_minute);

	/**
	 * Set climb rate visibility.
	 */
	void
	set_climb_rate_visibility (bool visible);

	/**
	 * Return speed bug value or 0.0f if not found.
	 */
	Knots
	speed_bug (QString name) const;

	/**
	 * Add new speed bug. A special value EFISWidget::AT ("A/T")
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
	 * Add new altitude bug. A special value EFISWidget::AP ("A/P")
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
	 * Set speed tendency value.
	 */
	void
	set_speed_tendency (KnotsPerSecond);

	/**
	 * Set speed tendency arrow visibility.
	 */
	void
	set_speed_tendency_visibility (bool visible);

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

	/**
	 * Set input alert visibility.
	 */
	void
	set_input_alert_visibility (bool visible);

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
	bool				_input_alert_visible		= false;
	TextPainter::Cache	_text_painter_cache;

	// Parameters:
	Degrees				_pitch						= 0.f;
	bool				_pitch_visibility			= false;
	Degrees				_roll						= 0.f;
	bool				_roll_visibility			= false;
	Degrees				_heading					= 0.f;
	bool				_heading_visibility			= false;
	Degrees				_flight_path_alpha			= 0.f;
	Degrees				_flight_path_beta			= 0.f;
	bool				_flight_path_visible		= false;
	Knots				_speed						= 0.f;
	bool				_speed_visible				= false;
	Feet				_altitude					= 0.f;
	bool				_altitude_visible			= false;
	FeetPerMinute		_climb_rate					= 0.f;
	bool				_climb_rate_visible			= false;
	SpeedBugs			_speed_bugs;
	AltitudeBugs		_altitude_bugs;
	float				_mach						= 0.f;
	bool				_mach_visible				= false;
	InHg				_pressure					= 0.f;
	bool				_pressure_visible			= false;
	Knots				_minimum_speed				= 0.f;
	bool				_minimum_speed_visible		= false;
	Knots				_warning_speed				= 0.f;
	bool				_warning_speed_visible		= false;
	Knots				_maximum_speed				= 0.f;
	bool				_maximum_speed_visible		= false;
	KnotsPerSecond		_speed_tendency				= 0.f;
	bool				_speed_tendency_visible		= false;

	static const char	DIGITS[];
	static const char*	MINUS_SIGN;
};


inline float
EFISWidget::AltitudeLadder::ft_to_px (Feet ft) const
{
	return -0.5f * _ladder_rect.height() * (ft - _altitude) / (_extent / 2.f);
}


inline float
EFISWidget::SpeedLadder::kt_to_px (Knots kt) const
{
	return -0.5f * _ladder_rect.height() * (kt - _speed) / (_extent / 2.f);
}


inline float
EFISWidget::AttitudeDirectorIndicator::pitch_to_px (Degrees degrees) const
{
	float const correction = 0.775f;
	return -degrees / (_efis._fov * correction) * _efis.wh();
}


inline float
EFISWidget::AttitudeDirectorIndicator::heading_to_px (Degrees degrees) const
{
	return pitch_to_px (-degrees);
}


inline Degrees
EFISWidget::pitch() const
{
	return _pitch;
}


inline void
EFISWidget::set_pitch (Degrees degrees)
{
	_pitch = degrees;
	update();
}


inline void
EFISWidget::set_pitch_visibility (bool visible)
{
	_pitch_visibility = visible;
	update();
}


inline Degrees
EFISWidget::roll() const
{
	return _roll;
}


inline void
EFISWidget::set_roll (Degrees degrees)
{
	_roll = degrees;
	update();
}


inline void
EFISWidget::set_roll_visibility (bool visible)
{
	_roll_visibility = visible;
	update();
}


inline Degrees
EFISWidget::heading() const
{
	return _heading;
}


inline void
EFISWidget::set_heading (Degrees degrees)
{
	_heading = degrees;
	update();
}


inline void
EFISWidget::set_heading_visibility (bool visible)
{
	_heading_visibility = visible;
	update();
}


inline Degrees
EFISWidget::flight_path_alpha() const
{
	return _flight_path_alpha;
}


inline void
EFISWidget::set_flight_path_alpha (Degrees pitch)
{
	_flight_path_alpha = pitch;
	update();
}


inline Degrees
EFISWidget::flight_path_beta() const
{
	return _flight_path_beta;
}


inline void
EFISWidget::set_flight_path_beta (Degrees heading)
{
	_flight_path_beta = heading;
	update();
}


inline void
EFISWidget::set_flight_path_marker_visibility (bool visible)
{
	_flight_path_visible = visible;
	update();
}


inline Knots
EFISWidget::speed() const
{
	return _speed;
}


inline void
EFISWidget::set_speed (Knots speed)
{
	_speed = speed;
	update();
}


inline void
EFISWidget::set_speed_visibility (bool visible)
{
	_speed_visible = visible;
	update();
}


inline Feet
EFISWidget::altitude() const
{
	return _altitude;
}


inline void
EFISWidget::set_altitude (Feet altitude)
{
	_altitude = altitude;
	update();
}


inline void
EFISWidget::set_altitude_visibility (bool visible)
{
	_altitude_visible = visible;
	update();
}


inline FeetPerMinute
EFISWidget::climb_rate() const
{
	return _climb_rate;
}


inline void
EFISWidget::set_climb_rate (FeetPerMinute feet_per_minute)
{
	_climb_rate = feet_per_minute;
	update();
}


inline void
EFISWidget::set_climb_rate_visibility (bool visible)
{
	_climb_rate_visible = visible;
	update();
}


inline Knots
EFISWidget::speed_bug (QString name) const
{
	auto it = _speed_bugs.find (name);
	if (it != _speed_bugs.end())
		return it->second;
	return 0.f;
}


inline void
EFISWidget::add_speed_bug (QString name, Knots speed)
{
	_speed_bugs[name] = speed;
	update();
}


inline void
EFISWidget::remove_speed_bug (QString name)
{
	if (name.isNull())
		_speed_bugs.clear();
	else
		_speed_bugs.erase (name);
	update();
}


inline Feet
EFISWidget::altitude_bug (QString name) const
{
	auto it = _altitude_bugs.find (name);
	if (it != _altitude_bugs.end())
		return it->second;
	return 0.f;
}


inline void
EFISWidget::add_altitude_bug (QString name, Feet altitude)
{
	_altitude_bugs[name] = altitude;
	update();
}


inline void
EFISWidget::remove_altitude_bug (QString name)
{
	if (name.isNull())
		_altitude_bugs.clear();
	else
		_altitude_bugs.erase (name);
	update();
}


inline float
EFISWidget::mach() const
{
	return _mach;
}


inline void
EFISWidget::set_mach (float value)
{
	_mach = value;
	update();
}


inline void
EFISWidget::set_mach_visibility (bool visible)
{
	_mach_visible = visible;
	update();
}


inline InHg
EFISWidget::pressure() const
{
	return _pressure;
}


inline void
EFISWidget::set_pressure (InHg pressure)
{
	_pressure = pressure;
	update();
}


inline void
EFISWidget::set_pressure_visibility (bool visible)
{
	_pressure_visible = visible;
	update();
}


inline Knots
EFISWidget::minimum_speed() const
{
	return _minimum_speed;
}


inline void
EFISWidget::set_minimum_speed (Knots minimum_speed)
{
	_minimum_speed = minimum_speed;
	update();
}


inline void
EFISWidget::set_minimum_speed_visibility (bool visible)
{
	_minimum_speed_visible = visible;
	update();
}


inline Knots
EFISWidget::warning_speed() const
{
	return _warning_speed;
}


inline void
EFISWidget::set_warning_speed (Knots warning_speed)
{
	_warning_speed = warning_speed;
	update();
}


inline void
EFISWidget::set_warning_speed_visibility (bool visible)
{
	_warning_speed_visible = visible;
	update();
}


inline Knots
EFISWidget::maximum_speed() const
{
	return _maximum_speed;
}


inline void
EFISWidget::set_maximum_speed (Knots maximum_speed)
{
	_maximum_speed = maximum_speed;
	update();
}


inline void
EFISWidget::set_maximum_speed_visibility (bool visible)
{
	_maximum_speed_visible = visible;
	update();
}


inline void
EFISWidget::set_speed_tendency (KnotsPerSecond kps)
{
	_speed_tendency = kps;
	update();
}


inline void
EFISWidget::set_speed_tendency_visibility (bool visible)
{
	_speed_tendency_visible = visible;
	update();
}


inline Degrees
EFISWidget::fov() const
{
	return _fov;
}


inline void
EFISWidget::set_fov (Degrees degrees)
{
	_fov = degrees;
	update();
}


inline void
EFISWidget::set_input_alert_visibility (bool visible)
{
	_input_alert_visible = visible;
}


inline float
EFISWidget::wh() const
{
	return std::min (0.8f * width(), 1.f * height());
}


inline QPen
EFISWidget::get_pen (QColor const& color, float width)
{
	return QPen (color, pen_width (width), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}


inline float
EFISWidget::pen_width (float scale) const
{
	return std::max (0.f, scale * wh() / 325.f);
}


inline float
EFISWidget::font_size (float scale) const
{
	return std::max (1.f, scale * wh() / 375.f);
}

#endif

