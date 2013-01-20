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

#ifndef XEFIS__WIDGETS__EFIS_WIDGET_H__INCLUDED
#define XEFIS__WIDGETS__EFIS_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Qt:
#include <QtGui/QPaintEvent>
#include <QtGui/QColor>
#include <QtGui/QPainterPath>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument_widget.h>
#include <xefis/utility/text_painter.h>


class EFISWidget: public Xefis::InstrumentWidget
{
	Q_OBJECT

	typedef std::map<QString, Knots> SpeedBugs;
	typedef std::map<QString, Feet> AltitudeBugs;

  public:
	// Ctor
	EFISWidget (QWidget* parent);

	/**
	 * Set how often lines should be drawn on speed ladder.
	 */
	void
	set_speed_ladder_line_every (int knots);

	/**
	 * Set how often numbers should be drawn on speed ladder.
	 */
	void
	set_speed_ladder_number_every (int knots);

	/**
	 * Set speed ladder scale extent.
	 */
	void
	set_speed_ladder_extent (int knots);

	/**
	 * Set how often lines should be drawn on altitude ladder.
	 */
	void
	set_altitude_ladder_line_every (int feet);

	/**
	 * Set how often numbers should be drawn on altitude ladder.
	 */
	void
	set_altitude_ladder_number_every (int feet);

	/**
	 * Set how often lines should be drawn bold on altitude ladder.
	 */
	void
	set_altitude_ladder_bold_every (int feet);

	/**
	 * Set altitude ladder scale extent.
	 */
	void
	set_altitude_ladder_extent (int feet);

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
	set_pitch_visible (bool visible);

	/**
	 * Return current roll value.
	 */
	Degrees
	roll() const;

	/**
	 * Return roll limit.
	 */
	Degrees
	roll_limit() const;

	/**
	 * Set roll value.
	 */
	void
	set_roll (Degrees);

	/**
	 * Set roll limit, where indicator will turn solid amber.
	 * 0.f disables the limit.
	 */
	void
	set_roll_limit (Degrees limit);

	/**
	 * Toggle roll scale visibility.
	 * Toggles also artifical horizon.
	 */
	void
	set_roll_visible (bool visible);

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
	 * Toggle heading scale numbers visibility (only on ADI, not on NAV widget).
	 */
	void
	set_heading_numbers_visible (bool visible);

	/**
	 * Return slip-skid indicator value.
	 */
	float
	slip_skid() const;

	/**
	 * Return slip-skid limit.
	 */
	float
	slip_skid_limit() const;

	/**
	 * Set slip-skid value.
	 */
	void
	set_slip_skid (float value);

	/**
	 * Set slip-skid limit, where indicator will turn solid amber.
	 * 0.f disables the limit.
	 */
	void
	set_slip_skid_limit (float limit);

	/**
	 * Set slip-skid indicator visibility.
	 */
	void
	set_slip_skid_visible (bool visible);

	/**
	 * Flight path vertical deviation.
	 */
	Degrees
	flight_path_alpha() const;

	/**
	 * Set flight path vertical deviation.
	 */
	void
	set_flight_path_alpha (Degrees);

	/**
	 * Flight path horizontal deviation.
	 */
	Degrees
	flight_path_beta() const;

	/**
	 * Set flight path horizontal deviation.
	 */
	void
	set_flight_path_beta (Degrees);

	/**
	 * Set visibility of the Flight Path Marker.
	 */
	void
	set_flight_path_marker_visible (bool visible);

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
	set_speed_visible (bool visible);

	/**
	 * Set speed tendency value.
	 */
	void
	set_speed_tendency (Knots);

	/**
	 * Set speed tendency arrow visibility.
	 */
	void
	set_speed_tendency_visible (bool visible);

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
	 * Set altitude tendency value.
	 */
	void
	set_altitude_tendency (Feet);

	/**
	 * Set altitude tendency arrow visibility.
	 */
	void
	set_altitude_tendency_visible (bool visible);

	/**
	 * Set speed tendency value.
	 */
	void
	set_altitude_agl (Feet);

	/**
	 * Set speed tendency arrow visibility.
	 */
	void
	set_altitude_agl_visible (bool visible);

	/**
	 * Return landing altitude.
	 */
	Feet
	landing_altitude() const;

	/**
	 * Set landing altitude.
	 */
	void
	set_landing_altitude (Feet);

	/**
	 * Set landing altitude visibility.
	 */
	void
	set_landing_altitude_visible (bool visible);

	/**
	 * Toggle visibility of the altitude scale.
	 */
	void
	set_altitude_visible (bool visible);

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
	set_climb_rate_visible (bool visible);

	/**
	 * Return speed bug value or 0.0f if not found.
	 */
	Knots
	speed_bug (QString name) const;

	/**
	 * Add new speed bug.
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
	 * Add new altitude bug.
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
	set_mach_visible (bool visible);

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
	set_pressure_visible (bool visible);

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
	set_minimum_speed_visible (bool visible);

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
	set_warning_speed_visible (bool visible);

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
	set_maximum_speed_visible (bool visible);

	/**
	 * Return autopilot altitude.
	 */
	Feet
	ap_altitude() const;

	/**
	 * Set autopilot altitude.
	 */
	void
	set_ap_altitude (Feet);

	/**
	 * Set AP altitude setting visibility.
	 */
	void
	set_ap_altitude_visible (bool visible);

	/**
	 * Return autopilot climb rate setting.
	 */
	FeetPerMinute
	ap_climb_rate() const;

	/**
	 * Set autopilot climb rate setting.
	 */
	void
	set_ap_climb_rate (FeetPerMinute);

	/**
	 * Set AP climb rate visibility.
	 */
	void
	set_ap_climb_rate_visible (bool visible);

	/**
	 * Return autothrottle setting.
	 */
	Knots
	at_speed() const;

	/**
	 * Set autothrottle speed.
	 */
	void
	set_at_speed (Knots);

	/**
	 * Set AT speed visibility.
	 */
	void
	set_at_speed_visible (bool visible);

	/**
	 * Return flight director alpha.
	 */
	Degrees
	flight_director_pitch() const;

	/**
	 * Set flight director pitch.
	 */
	void
	set_flight_director_pitch (Degrees pitch);

	/**
	 * Set flight director pitch visibility.
	 */
	void
	set_flight_director_pitch_visible (bool visible);

	/**
	 * Return flight director roll.
	 */
	Degrees
	flight_director_roll() const;

	/**
	 * Set flight director roll.
	 */
	void
	set_flight_director_roll (Degrees roll);

	/**
	 * Set flight director roll visibility.
	 */
	void
	set_flight_director_roll_visible (bool visible);

	/**
	 * Set visibility of navigation needles ladders.
	 */
	void
	set_navigation_needles_visible (bool visible);

	/**
	 * Return navigation glideslope needle value.
	 */
	float
	nav_glideslope_needle() const;

	/**
	 * Set navigation glideslope needle. Values: [-1.f, +1.f].
	 */
	void
	set_navigation_glideslope_needle (float value);

	/**
	 * Set navigation glideslope needle visibility.
	 */
	void
	set_navigation_glideslope_needle_visible (bool visible);

	/**
	 * Return navigation heading needle value.
	 */
	float
	nav_heading_needle() const;

	/**
	 * Set navigation heading needle. Values: [-1.f, +1.f].
	 */
	void
	set_navigation_heading_needle (float value);

	/**
	 * Set navigation heading needle visibility.
	 */
	void
	set_navigation_heading_needle_visible (bool visible);

	/**
	 * Set navigation runway visibility (aligns with heading needle).
	 */
	void
	set_navigation_runway_visible (bool visible);

	/**
	 * Return nav hint.
	 */
	QString
	nav_hint() const;

	/**
	 * Set navigation hint, a text shown on the top left corner of the ADI.
	 * Usually something like "ILS" or "VOR".
	 */
	void
	set_navigation_hint (QString hint);

	/**
	 * Return DME distance.
	 */
	Miles
	dme_distance() const;

	/**
	 * Set DME distance.
	 */
	void
	set_dme_distance (Miles);

	/**
	 * Set DME info visibility.
	 */
	void
	set_dme_distance_visible (bool visible);

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
	set_input_alert_visible (bool visible);

  protected slots:
	void
	blink();

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

	/*
	 * ADI
	 */

	void
	adi_post_resize();

	void
	adi_pre_paint();

	void
	adi_paint (QPainter&, TextPainter&);

	void
	adi_paint_horizon (QPainter&);

	void
	adi_paint_pitch (QPainter&, TextPainter&);

	void
	adi_paint_roll (QPainter&);

	void
	adi_paint_heading (QPainter&, TextPainter&);

	void
	adi_paint_flight_path_marker (QPainter&);

	/*
	 * Speed ladder
	 */

	void
	sl_post_resize();

	void
	sl_pre_paint();

	void
	sl_paint (QPainter&, TextPainter&);

	void
	sl_paint_black_box (QPainter& painter, TextPainter&, float x);

	void
	sl_paint_ladder_scale (QPainter& painter, TextPainter&, float x);

	void
	sl_paint_speed_limits (QPainter& painter, float x);

	void
	sl_paint_speed_tendency (QPainter& painter, float x);

	void
	sl_paint_bugs (QPainter& painter, TextPainter&, float x);

	void
	sl_paint_mach_number (QPainter& painter, TextPainter&, float x);

	void
	sl_paint_ap_setting (QPainter& painter, TextPainter&);

	float
	kt_to_px (Knots ft) const;

	/*
	 * Altitude ladder
	 */

	void
	al_post_resize();

	void
	al_pre_paint();

	void
	al_paint (QPainter&, TextPainter&);

	void
	al_paint_black_box (QPainter& painter, TextPainter& text_painter, float x);

	void
	al_paint_ladder_scale (QPainter& painter, TextPainter& text_painter, float x);

	void
	al_paint_altitude_tendency (QPainter& painter, float x);

	void
	al_paint_bugs (QPainter& painter, TextPainter& text_painter, float x);

	void
	al_paint_climb_rate (QPainter& painter, TextPainter& text_painter, float x);

	void
	al_paint_pressure (QPainter& painter, TextPainter& text_painter, float x);

	void
	al_paint_ap_setting (QPainter& painter, TextPainter& text_painter);

	float
	ft_to_px (Feet ft) const;

	float
	scale_cbr (FeetPerMinute climb_rate) const;

	/*
	 * Other
	 */

	void
	paint_center_cross (QPainter&);

	void
	paint_flight_director (QPainter&);

	void
	paint_altitude_agl (QPainter&, TextPainter&);

	void
	paint_nav (QPainter&, TextPainter&);

	void
	paint_input_alert (QPainter&, TextPainter&);

	void
	paint_dashed_zone (QPainter& painter, QColor const& color, QRectF const& target);

	/**
	 * Render 'rotatable' value on speed/altitude black box.
	 *
	 * \param	painter
	 * 			QPainter to use.
	 * \param	text_painter
	 * 			TextPainter to use.
	 * \param	position
	 * 			Text position, [-0.5, 0.5].
	 * \param	next, curr, prev
	 * 			Texts to render. Special value "G" paints green dashed zone, "R" paints red dashed zone.
	 */
	void
	paint_rotating_value (QPainter& painter, TextPainter& text_painter,
						  QRectF const& rect, float position, float height_scale,
						  QString const& next, QString const& curr, QString const& prev);

	/**
	 * \param	two_zeros
	 * 			Two separate zeros, for positive and negative values.
	 * \param	zero_mark
	 * 			Draw red/green mark instead of zero.
	 */
	void
	paint_rotating_digit (QPainter& painter, TextPainter& text_painter,
						  QRectF const& box, float value, int round_target, float const height_scale, float const delta, float const phase,
						  bool two_zeros, bool zero_mark);

  private:
	float
	pitch_to_px (Degrees degrees) const;

	float
	heading_to_px (Degrees degrees) const;

	QPainterPath
	get_pitch_scale_clipping_path() const;

  private:
	QColor				_sky_color;
	QColor				_ground_color;
	QColor				_ladder_color;
	QColor				_ladder_border_color;
	QColor				_warning_color_1;
	QColor				_warning_color_2;
	QTransform			_center_transform;
	QTransform			_pitch_transform;
	QTransform			_roll_transform;
	QTransform			_heading_transform;
	QTransform			_horizon_transform;
	Degrees				_fov							= 120.f;
	bool				_input_alert_visible			= false;
	TextPainter::Cache	_text_painter_cache;
	QTimer*				_blinking_warning				= nullptr;
	bool				_blink							= false;

	float				_w;
	float				_h;
	float				_max_w_h;

	/*
	 * ADI
	 */

	QRectF				_adi_sky_rect;
	QRectF				_adi_gnd_rect;
	QPainterPath		_flight_path_marker_clip;
	QPointF				_flight_path_marker_position;

	/*
	 * Speed ladder
	 */

	Knots				_sl_extent						= 124;
	int					_sl_line_every					= 10;
	int					_sl_number_every				= 20;
	Knots				_sl_min_shown;
	Knots				_sl_max_shown;
	int					_sl_rounded_speed;
	QRectF				_sl_ladder_rect;
	QPen				_sl_ladder_pen;
	QRectF				_sl_black_box_rect;
	QPen				_sl_black_box_pen;
	QPen				_sl_scale_pen;
	QPen				_sl_speed_bug_pen;
	float				_sl_margin;
	int					_sl_digits;

	/*
	 * Altitude ladder
	 */

	int					_al_line_every					= 100;
	int					_al_number_every				= 200;
	int					_al_bold_every					= 500;
	Feet				_al_extent						= 825;
	Feet				_al_min_shown;
	Feet				_al_max_shown;
	int					_al_rounded_altitude;
	QRectF				_al_ladder_rect;
	QPen				_al_ladder_pen;
	QRectF				_al_black_box_rect;
	QPen				_al_black_box_pen;
	QPen				_al_scale_pen_1;
	QPen				_al_scale_pen_2; // Bold one, each 500 ft
	QPen				_al_negative_altitude_pen;
	QPen				_al_altitude_bug_pen;
	QPen				_al_ldg_alt_pen;
	QRectF				_al_b_digits_box;
	QRectF				_al_s_digits_box;
	float				_al_margin;

	/*
	 * Parameters
	 */

	Degrees				_pitch							= 0.f;
	bool				_pitch_visible					= false;
	Degrees				_roll							= 0.f;
	Degrees				_roll_limit						= 0.f;
	bool				_roll_visible					= false;
	Degrees				_heading						= 0.f;
	bool				_heading_visible				= false;
	bool				_heading_numbers_visible		= false;
	float				_slip_skid						= 0.f;
	float				_slip_skid_limit				= 0.f;
	bool				_slip_skid_visible				= false;
	Degrees				_flight_path_alpha				= 0.f;
	Degrees				_flight_path_beta				= 0.f;
	bool				_flight_path_visible			= false;
	Knots				_speed							= 0.f;
	bool				_speed_visible					= false;
	Knots				_speed_tendency					= 0.f;
	bool				_speed_tendency_visible			= false;
	Feet				_altitude						= 0.f;
	bool				_altitude_visible				= false;
	Feet				_altitude_tendency				= 0.f;
	bool				_altitude_tendency_visible		= false;
	Feet				_altitude_agl					= 0.f;
	bool				_altitude_agl_visible			= false;
	Feet				_landing_altitude				= 0.f;
	bool				_landing_altitude_visible		= false;
	FeetPerMinute		_climb_rate						= 0.f;
	bool				_climb_rate_visible				= false;
	float				_mach							= 0.f;
	bool				_mach_visible					= false;
	InHg				_pressure						= 0.f;
	bool				_pressure_visible				= false;
	Knots				_minimum_speed					= 0.f;
	bool				_minimum_speed_visible			= false;
	Knots				_warning_speed					= 0.f;
	bool				_warning_speed_visible			= false;
	Knots				_maximum_speed					= 0.f;
	bool				_maximum_speed_visible			= false;
	Feet				_ap_altitude					= 0.f;
	bool				_ap_altitude_visible			= false;
	FeetPerMinute		_ap_climb_rate					= 0.f;
	bool				_ap_climb_rate_visible			= false;
	Knots				_at_speed						= 0.f;
	bool				_at_speed_visible				= false;
	Degrees				_flight_director_pitch			= 0.f;
	bool				_flight_director_pitch_visible	= false;
	Degrees				_flight_director_roll			= 0.f;
	bool				_flight_director_roll_visible	= false;
	bool				_navigation_needles_visible		= false;
	float				_navigation_gs_needle			= 0.f;
	bool				_navigation_gs_needle_visible	= false;
	float				_navigation_hd_needle			= 0.f;
	bool				_navigation_hd_needle_visible	= false;
	bool				_navigation_runway_visible		= false;
	QString				_navigation_hint;
	Miles				_dme_distance					= 0.f;
	bool				_dme_distance_visible			= false;
	SpeedBugs			_speed_bugs;
	AltitudeBugs		_altitude_bugs;
};


inline void
EFISWidget::set_speed_ladder_line_every (int knots)
{
	_sl_line_every = std::max (1, knots);
	update();
}


inline void
EFISWidget::set_speed_ladder_number_every (int knots)
{
	_sl_number_every = std::max (1, knots);
	update();
}


inline void
EFISWidget::set_speed_ladder_extent (int knots)
{
	_sl_extent = std::max (1, knots);
	update();
}


inline void
EFISWidget::set_altitude_ladder_line_every (int feet)
{
	_al_line_every = std::max (1, feet);
	update();
}


inline void
EFISWidget::set_altitude_ladder_number_every (int feet)
{
	_al_number_every = std::max (1, feet);
	update();
}


inline void
EFISWidget::set_altitude_ladder_bold_every (int feet)
{
	_al_bold_every = std::max (1, feet);
	update();
}


inline void
EFISWidget::set_altitude_ladder_extent (int feet)
{
	_al_extent = std::max (1, feet);
	update();
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
EFISWidget::set_pitch_visible (bool visible)
{
	_pitch_visible = visible;
	update();
}


inline Degrees
EFISWidget::roll() const
{
	return _roll;
}


inline Degrees
EFISWidget::roll_limit() const
{
	return _roll_limit;
}


inline void
EFISWidget::set_roll (Degrees degrees)
{
	_roll = degrees;
	update();
}


inline void
EFISWidget::set_roll_limit (Degrees limit)
{
	_roll_limit = limit;
	update();
}


inline void
EFISWidget::set_roll_visible (bool visible)
{
	_roll_visible = visible;
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
EFISWidget::set_heading_visible (bool visible)
{
	_heading_visible = visible;
	update();
}


inline void
EFISWidget::set_heading_numbers_visible (bool visible)
{
	_heading_numbers_visible = visible;
	update();
}


inline float
EFISWidget::slip_skid() const
{
	return _slip_skid;
}


inline float
EFISWidget::slip_skid_limit() const
{
	return _slip_skid_limit;
}


inline void
EFISWidget::set_slip_skid (float value)
{
	_slip_skid = value;
	update();
}


inline void
EFISWidget::set_slip_skid_limit (float limit)
{
	_slip_skid_limit = limit;
	update();
}


inline void
EFISWidget::set_slip_skid_visible (bool visible)
{
	_slip_skid_visible = visible;
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
EFISWidget::set_flight_path_marker_visible (bool visible)
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
EFISWidget::set_speed_visible (bool visible)
{
	_speed_visible = visible;
	update();
}


inline void
EFISWidget::set_speed_tendency (Knots kt)
{
	_speed_tendency = kt;
	update();
}


inline void
EFISWidget::set_speed_tendency_visible (bool visible)
{
	_speed_tendency_visible = visible;
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
EFISWidget::set_altitude_visible (bool visible)
{
	_altitude_visible = visible;
	update();
}


inline void
EFISWidget::set_altitude_tendency (Feet ft)
{
	_altitude_tendency = ft;
	update();
}


inline void
EFISWidget::set_altitude_tendency_visible (bool visible)
{
	_altitude_tendency_visible = visible;
	update();
}


inline void
EFISWidget::set_altitude_agl (Feet altitude)
{
	_altitude_agl = altitude;
	update();
}


inline void
EFISWidget::set_altitude_agl_visible (bool visible)
{
	_altitude_agl_visible = visible;
	update();
}


inline Feet
EFISWidget::landing_altitude() const
{
	return _landing_altitude;
}


inline void
EFISWidget::set_landing_altitude (Feet feet)
{
	_landing_altitude = feet;
	update();
}


inline void
EFISWidget::set_landing_altitude_visible (bool visible)
{
	_landing_altitude_visible = visible;
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
EFISWidget::set_climb_rate_visible (bool visible)
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
EFISWidget::set_mach_visible (bool visible)
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
EFISWidget::set_pressure_visible (bool visible)
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
EFISWidget::set_minimum_speed_visible (bool visible)
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
EFISWidget::set_warning_speed_visible (bool visible)
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
EFISWidget::set_maximum_speed_visible (bool visible)
{
	_maximum_speed_visible = visible;
	update();
}


inline Feet
EFISWidget::ap_altitude() const
{
	return _ap_altitude;
}


inline void
EFISWidget::set_ap_altitude (Feet feet)
{
	_ap_altitude = feet;
	update();
}


inline void
EFISWidget::set_ap_altitude_visible (bool visible)
{
	_ap_altitude_visible = visible;
	update();
}


inline FeetPerMinute
EFISWidget::ap_climb_rate() const
{
	return _ap_climb_rate;
}


inline void
EFISWidget::set_ap_climb_rate (FeetPerMinute fpm)
{
	_ap_climb_rate = fpm;
	update();
}


inline void
EFISWidget::set_ap_climb_rate_visible (bool visible)
{
	_ap_climb_rate_visible = visible;
	update();
}


inline Knots
EFISWidget::at_speed() const
{
	return _at_speed;
}


inline void
EFISWidget::set_at_speed (Knots knots)
{
	_at_speed = knots;
	update();
}


inline void
EFISWidget::set_at_speed_visible (bool visible)
{
	_at_speed_visible = visible;
	update();
}


inline Degrees
EFISWidget::flight_director_pitch() const
{
	return _flight_director_pitch;
}


inline void
EFISWidget::set_flight_director_pitch (Degrees pitch)
{
	_flight_director_pitch = pitch;
	update();
}


inline void
EFISWidget::set_flight_director_pitch_visible (bool visible)
{
	_flight_director_pitch_visible = visible;
	update();
}


inline Degrees
EFISWidget::flight_director_roll() const
{
	return _flight_director_roll;
}


inline void
EFISWidget::set_flight_director_roll (Degrees roll)
{
	_flight_director_roll = roll;
	update();
}


inline void
EFISWidget::set_flight_director_roll_visible (bool visible)
{
	_flight_director_roll_visible = visible;
	update();
}


inline void
EFISWidget::set_navigation_needles_visible (bool visible)
{
	_navigation_needles_visible = visible;
	update();
}


inline float
EFISWidget::nav_glideslope_needle() const
{
	return _navigation_gs_needle;
}


inline void
EFISWidget::set_navigation_glideslope_needle (float value)
{
	_navigation_gs_needle = value;
	update();
}


inline void
EFISWidget::set_navigation_glideslope_needle_visible (bool visible)
{
	_navigation_gs_needle_visible = visible;
	update();
}


inline float
EFISWidget::nav_heading_needle() const
{
	return _navigation_hd_needle;
}


inline void
EFISWidget::set_navigation_heading_needle (float value)
{
	_navigation_hd_needle = value;
	update();
}


inline void
EFISWidget::set_navigation_heading_needle_visible (bool visible)
{
	_navigation_hd_needle_visible = visible;
	update();
}


inline void
EFISWidget::set_navigation_runway_visible (bool visible)
{
	_navigation_runway_visible = visible;
	update();
}


inline QString
EFISWidget::nav_hint() const
{
	return _navigation_hint;
}


inline void
EFISWidget::set_navigation_hint (QString hint)
{
	_navigation_hint = hint;
	update();
}


inline Miles
EFISWidget::dme_distance() const
{
	return _dme_distance;
}


inline void
EFISWidget::set_dme_distance (Miles distance)
{
	_dme_distance = distance;
	update();
}


inline void
EFISWidget::set_dme_distance_visible (bool visible)
{
	_dme_distance_visible = visible;
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
EFISWidget::set_input_alert_visible (bool visible)
{
	_input_alert_visible = visible;
}


inline float
EFISWidget::kt_to_px (Knots kt) const
{
	return -0.5f * _sl_ladder_rect.height() * (kt - _speed) / (0.5f * _sl_extent);
}


inline float
EFISWidget::ft_to_px (Feet ft) const
{
	return -0.5f * _al_ladder_rect.height() * (ft - _altitude) / (0.5f * _al_extent);
}


inline void
EFISWidget::blink()
{
	_blink = !_blink;
}


inline float
EFISWidget::pitch_to_px (Degrees degrees) const
{
	float const correction = 0.775f;
	return -degrees / (_fov * correction) * wh();
}


inline float
EFISWidget::heading_to_px (Degrees degrees) const
{
	return pitch_to_px (-degrees);
}

#endif

