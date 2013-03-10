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

#ifndef XEFIS__MODULES__INSTRUMENTS__EFIS_WIDGET_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__EFIS_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Qt:
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QPaintEvent>
#include <QtGui/QColor>
#include <QtGui/QPainterPath>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument_widget.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/utility/text_painter.h>


class EFISWidget:
	public Xefis::InstrumentWidget,
	public Xefis::InstrumentAids
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
	 * Set speed ladder lowest value.
	 */
	void
	set_speed_ladder_minimum (int knots);

	/**
	 * Set speed ladder highest value.
	 */
	void
	set_speed_ladder_maximum (int knots);

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
	 * Set pitch value.
	 */
	void
	set_pitch (Angle);

	/**
	 * Toggle pitch scale visibility.
	 * Toggles also artifical horizon.
	 */
	void
	set_pitch_visible (bool visible);

	/**
	 * Set pitch limit (absolute value).
	 */
	void
	set_pitch_limit (Angle pitch_limit);

	/**
	 * Set pitch limit indicator visibility.
	 */
	void
	set_pitch_limit_visible (bool visible);

	/**
	 * Set roll value.
	 */
	void
	set_roll (Angle);

	/**
	 * Set roll limit, where indicator will turn solid amber.
	 * 0.f disables the limit.
	 */
	void
	set_roll_limit (Angle limit);

	/**
	 * Toggle roll scale visibility.
	 * Toggles also artifical horizon.
	 */
	void
	set_roll_visible (bool visible);

	/**
	 * Set heading value.
	 */
	void
	set_heading (Angle);

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
	 * Set flight path vertical deviation.
	 */
	void
	set_flight_path_alpha (Angle);

	/**
	 * Set flight path horizontal deviation.
	 */
	void
	set_flight_path_beta (Angle);

	/**
	 * Set visibility of the Flight Path Marker.
	 */
	void
	set_flight_path_marker_visible (bool visible);

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
	 * Set visibility of the NO VSPD (no V-speeds)
	 * flag.
	 */
	void
	set_novspd_flag (bool visible);

	/**
	 * Set altitude value.
	 */
	void
	set_altitude (Feet);

	/**
	 * Toggle visibility of the altitude scale.
	 */
	void
	set_altitude_visible (bool visible);

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
	 * Set radar altitude.
	 */
	void
	set_altitude_agl (Feet);

	/**
	 * Set radar altitude visibility.
	 */
	void
	set_altitude_agl_visible (bool visible);

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
	 * Set visibility of the altitude warnings (500 and 1000 ft) above
	 * max of AGL altitude and LDG altitude or 0.
	 */
	void
	set_altitude_warnings_visible (bool visible);

	/**
	 * Set transition altitude.
	 */
	void
	set_transition_altitude (Feet);

	/**
	 * Set transition altitude visibility.
	 */
	void
	set_transition_altitude_visible (bool visible);

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
	 * Set pressure indicator.
	 */
	void
	set_pressure (Pressure pressure);

	/**
	 * Set pressure unit to be hPa instead of inHg.
	 */
	void
	set_pressure_display_hpa (bool hpa);

	/**
	 * Show or hide pressure indicator.
	 */
	void
	set_pressure_visible (bool visible);

	/**
	 * Enable/disable standard pressure.
	 */
	void
	set_standard_pressure (bool standard);

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
	 * Set commanded altitude.
	 */
	void
	set_cmd_altitude (Feet);

	/**
	 * Set AP altitude setting visibility.
	 */
	void
	set_cmd_altitude_visible (bool visible);

	/**
	 * Set commanded climb rate setting.
	 */
	void
	set_cmd_climb_rate (FeetPerMinute);

	/**
	 * Set AP climb rate visibility.
	 */
	void
	set_cmd_climb_rate_visible (bool visible);

	/**
	 * Set autothrottle speed.
	 */
	void
	set_cmd_speed (Knots);

	/**
	 * Set AT speed visibility.
	 */
	void
	set_cmd_speed_visible (bool visible);

	/**
	 * Set flight director pitch.
	 */
	void
	set_flight_director_pitch (Angle pitch);

	/**
	 * Set flight director pitch visibility.
	 */
	void
	set_flight_director_pitch_visible (bool visible);

	/**
	 * Set flight director roll.
	 */
	void
	set_flight_director_roll (Angle roll);

	/**
	 * Set flight director roll visibility.
	 */
	void
	set_flight_director_roll_visible (bool visible);

	/**
	 * Set control stick indicator pitch.
	 */
	void
	set_control_stick_pitch (Angle pitch);

	/**
	 * Set control stick indicator roll.
	 */
	void
	set_control_stick_roll (Angle roll);

	/**
	 * Set visibility of the control stick indicator.
	 */
	void
	set_control_stick_visible (bool visible);

	/**
	 * Set visibility of approach reference info (localizer/glideslope needles,
	 * localizer ID/bearing, DME, etc).
	 */
	void
	set_approach_reference_visible (bool visible);

	/**
	 * Set vertical deviation.
	 */
	void
	set_vertical_deviation (Angle deviation);

	/**
	 * Set navigation vertical needle visibility.
	 */
	void
	set_vertical_deviation_visible (bool visible);

	/**
	 * Set localizer deviation needle.
	 */
	void
	set_lateral_deviation (Angle value);

	/**
	 * Set navigation heading needle visibility.
	 */
	void
	set_lateral_deviation_visible (bool visible);

	/**
	 * Set runway visibility (aligns with lateral deviation needle).
	 */
	void
	set_runway_visible (bool visible);

	/**
	 * Set runway position relative to the horizon.
	 */
	void
	set_runway_position (Angle position);

	/**
	 * Set navigation hint, a text shown on the top left corner of the ADI.
	 * Usually something like "ILS" or "VOR".
	 */
	void
	set_approach_hint (QString hint);

	/**
	 * Set DME distance.
	 */
	void
	set_dme_distance (Length);

	/**
	 * Set DME info visibility.
	 */
	void
	set_dme_distance_visible (bool visible);

	/**
	 * Set localizer ID.
	 */
	void
	set_localizer_id (QString const& loc_id);

	/**
	 * Set localizer magnetic bearing.
	 */
	void
	set_localizer_magnetic_bearing (Angle mag_bearing);

	/**
	 * Set visibility of localizer ID and its bearing.
	 */
	void
	set_localizer_info_visible (bool visible);

	/**
	 * Set control hint - the text displayed right above roll scale.
	 */
	void
	set_control_hint (QString const&);

	/**
	 * Set visibility of the control hint.
	 */
	void
	set_control_hint_visible (bool visible);

	/**
	 * Set FMA (Flight mode annunciator) visibility.
	 */
	void
	set_fma_visible (bool visible);

	/**
	 * Set AP speed hint text.
	 */
	void
	set_fma_speed_hint (QString const&);

	/**
	 * Set additional AP speed hint text.
	 */
	void
	set_fma_speed_small_hint (QString const&);

	/**
	 * Set AP lateral hint text.
	 */
	void
	set_fma_lateral_hint (QString const&);

	/**
	 * Set additional AP lateral hint text.
	 */
	void
	set_fma_lateral_small_hint (QString const&);

	/**
	 * Set AP altitude hint text.
	 */
	void
	set_fma_vertical_hint (QString const&);

	/**
	 * Set additional AP altitude hint text.
	 */
	void
	set_fma_vertical_small_hint (QString const&);

	/**
	 * Set field of view.
	 */
	void
	set_fov (Angle);

	/**
	 * Set input alert visibility.
	 */
	void
	set_input_alert_visible (bool visible);

  private slots:
	void
	blink_speed();

	void
	blink_baro();

  private:
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
	sl_paint_black_box (QPainter&, TextPainter&, float x);

	void
	sl_paint_ladder_scale (QPainter&, TextPainter&, float x);

	void
	sl_paint_speed_limits (QPainter&, float x);

	void
	sl_paint_speed_tendency (QPainter&, float x);

	void
	sl_paint_bugs (QPainter&, TextPainter&, float x);

	void
	sl_paint_mach_number (QPainter&, TextPainter&, float x);

	void
	sl_paint_ap_setting (QPainter&, TextPainter&);

	void
	sl_paint_novspd (QPainter&, TextPainter&);

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
	al_paint_black_box (QPainter&, TextPainter&, float x);

	void
	al_paint_ladder_scale (QPainter&, TextPainter&, float x);

	void
	al_paint_altitude_tendency (QPainter&, float x);

	void
	al_paint_bugs (QPainter&, TextPainter&, float x);

	void
	al_paint_climb_rate (QPainter&, TextPainter&, float x);

	void
	al_paint_pressure (QPainter&, TextPainter&, float x);

	void
	al_paint_ap_setting (QPainter&, TextPainter&);

	float
	ft_to_px (Feet ft) const;

	float
	scale_cbr (FeetPerMinute climb_rate) const;

	/*
	 * Other
	 */

	void
	paint_center_cross (QPainter&, bool center_box, bool rest);

	void
	paint_flight_director (QPainter&);

	void
	paint_control_stick (QPainter&);

	void
	paint_altitude_agl (QPainter&, TextPainter&);

	void
	paint_baro_setting (QPainter&, TextPainter&);

	void
	paint_nav (QPainter&, TextPainter&);

	void
	paint_hints (QPainter&, TextPainter&);

	void
	paint_pitch_limit (QPainter&);

	void
	paint_input_alert (QPainter&, TextPainter&);

	void
	paint_dashed_zone (QPainter&, QColor const&, QRectF const& target);

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
	 * 			Draw red/green/blank mark instead of zero.
	 */
	void
	paint_rotating_digit (QPainter& painter, TextPainter& text_painter,
						  QRectF const& box, float value, int round_target, float const height_scale, float const delta, float const phase,
						  bool two_zeros, bool zero_mark, bool black_zero = false);

	/**
	 * Start or stop blinking warning timer on given condition.
	 */
	void
	update_blinker (QTimer* warning_timer, bool condition, bool* blink_state);

	float
	pitch_to_px (Angle degrees) const;

	float
	heading_to_px (Angle degrees) const;

	QPainterPath
	get_pitch_scale_clipping_path() const;

	QColor
	get_baro_color() const;

	bool
	is_newly_set (QDateTime const& timestamp) const;

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
	Angle				_fov							= 120_deg;
	bool				_input_alert_visible			= false;
	TextPainter::Cache	_text_painter_cache;
	QTimer*				_speed_blinking_warning			= nullptr;
	bool				_speed_blink					= false;
	QTimer*				_baro_blinking_warning			= nullptr;
	bool				_baro_blink						= false;
	QDateTime			_current_datetime;

	float				_w;
	float				_h;
	float				_max_w_h;
	float				_q;

	/*
	 * ADI
	 */

	QRectF				_adi_sky_rect;
	QRectF				_adi_gnd_rect;
	QPainterPath		_flight_path_marker_shape;
	QPainterPath		_flight_path_marker_clip;
	QPointF				_flight_path_marker_position;

	/*
	 * Speed ladder
	 */

	QTransform			_sl_transform;
	Knots				_sl_extent						= 124;
	int					_sl_minimum						= 0;
	int					_sl_maximum						= 9999;
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

	QTransform			_al_transform;
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

	Angle				_pitch							= 0_deg;
	Angle				_pitch_limit					= 0_deg;
	bool				_pitch_visible					= false;
	bool				_pitch_limit_visible			= false;
	Angle				_roll							= 0_deg;
	Angle				_roll_limit						= 0_deg;
	bool				_roll_visible					= false;
	Angle				_heading						= 0_deg;
	bool				_heading_visible				= false;
	bool				_heading_numbers_visible		= false;
	float				_slip_skid						= 0.f;
	float				_slip_skid_limit				= 0.f;
	bool				_slip_skid_visible				= false;
	Angle				_flight_path_alpha				= 0_deg;
	Angle				_flight_path_beta				= 0_deg;
	bool				_flight_path_visible			= false;
	Knots				_speed							= 0.f;
	bool				_speed_visible					= false;
	Knots				_speed_tendency					= 0.f;
	bool				_speed_tendency_visible			= false;
	bool				_novspd_flag					= false;
	Feet				_altitude						= 0.f;
	bool				_altitude_visible				= false;
	Feet				_altitude_tendency				= 0.f;
	bool				_altitude_tendency_visible		= false;
	Feet				_altitude_agl					= 0.f;
	bool				_altitude_agl_visible			= false;
	QDateTime			_altitude_agl_ts;
	Feet				_landing_altitude				= 0.f;
	bool				_landing_altitude_visible		= false;
	bool				_altitude_warnings_visible		= false;
	Feet				_transition_altitude			= 0.f;
	bool				_transition_altitude_visible	= false;
	QDateTime			_transition_altitude_ts;
	FeetPerMinute		_climb_rate						= 0.f;
	bool				_climb_rate_visible				= false;
	float				_mach							= 0.f;
	bool				_mach_visible					= false;
	Pressure			_pressure						= 0_inhg;
	bool				_pressure_display_hpa			= false;
	bool				_pressure_visible				= false;
	bool				_standard_pressure				= false;
	Knots				_minimum_speed					= 0.f;
	bool				_minimum_speed_visible			= false;
	Knots				_warning_speed					= 0.f;
	bool				_warning_speed_visible			= false;
	Knots				_maximum_speed					= 0.f;
	bool				_maximum_speed_visible			= false;
	Feet				_cmd_altitude					= 0.f;
	bool				_cmd_altitude_visible			= false;
	FeetPerMinute		_cmd_climb_rate					= 0.f;
	bool				_cmd_climb_rate_visible			= false;
	Knots				_cmd_speed						= 0.f;
	bool				_cmd_speed_visible				= false;
	Angle				_flight_director_pitch			= 0_deg;
	bool				_flight_director_pitch_visible	= false;
	Angle				_flight_director_roll			= 0_deg;
	bool				_flight_director_roll_visible	= false;
	Angle				_control_stick_pitch			= 0_deg;
	Angle				_control_stick_roll				= 0_deg;
	bool				_control_stick_visible			= false;
	bool				_approach_reference_visible		= false;
	Angle				_vertical_deviation_deg			= 0_deg;
	bool				_vertical_deviation_visible		= false;
	Angle				_lateral_deviation_deg			= 0_deg;
	bool				_lateral_deviation_visible		= false;
	bool				_runway_visible					= false;
	Angle				_runway_position				= 0_deg;
	QString				_approach_hint;
	Length				_dme_distance					= 0_nm;
	bool				_dme_distance_visible			= false;
	QString				_localizer_id;
	Angle				_localizer_magnetic_bearing		= 0_deg;
	bool				_localizer_info_visible			= false;
	QString				_control_hint;
	bool				_control_hint_visible			= false;
	QDateTime			_control_hint_ts;
	bool				_fma_visible					= false;
	QString				_fma_speed_hint;
	QDateTime			_fma_speed_ts;
	QString				_fma_speed_small_hint;
	QDateTime			_fma_speed_small_ts;
	QString				_fma_lateral_hint;
	QDateTime			_fma_lateral_ts;
	QString				_fma_lateral_small_hint;
	QDateTime			_fma_lateral_small_ts;
	QString				_fma_vertical_hint;
	QDateTime			_fma_vertical_ts;
	QString				_fma_vertical_small_hint;
	QDateTime			_fma_vertical_small_ts;
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
EFISWidget::set_speed_ladder_minimum (int knots)
{
	_sl_minimum = std::max (0, knots);
}


inline void
EFISWidget::set_speed_ladder_maximum (int knots)
{
	_sl_maximum = std::min (9999, knots);
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


inline void
EFISWidget::set_pitch (Angle degrees)
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


inline void
EFISWidget::set_pitch_limit (Angle pitch_limit)
{
	_pitch_limit = pitch_limit;
	update();
}


inline void
EFISWidget::set_pitch_limit_visible (bool visible)
{
	_pitch_limit_visible = visible;
	update();
}


inline void
EFISWidget::set_roll (Angle degrees)
{
	_roll = degrees;
	update();
}


inline void
EFISWidget::set_roll_limit (Angle limit)
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


inline void
EFISWidget::set_heading (Angle degrees)
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


inline void
EFISWidget::set_flight_path_alpha (Angle pitch)
{
	_flight_path_alpha = pitch;
	update();
}


inline void
EFISWidget::set_flight_path_beta (Angle heading)
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


inline void
EFISWidget::set_novspd_flag (bool visible)
{
	_novspd_flag = visible;
	update();
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
	if (!_altitude_agl_visible && visible)
		_altitude_agl_ts = QDateTime::currentDateTime();
	_altitude_agl_visible = visible;
	update();
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


inline void
EFISWidget::set_altitude_warnings_visible (bool visible)
{
	_altitude_warnings_visible = visible;
	update();
}


inline void
EFISWidget::set_transition_altitude (Feet transition_altitude)
{
	if (_transition_altitude != transition_altitude)
		_transition_altitude_ts = QDateTime::currentDateTime();
	_transition_altitude = transition_altitude;
	update();
}


inline void
EFISWidget::set_transition_altitude_visible (bool visible)
{
	if (_transition_altitude_visible != visible)
		_transition_altitude_ts = QDateTime::currentDateTime();
	_transition_altitude_visible = visible;
	update();
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


inline void
EFISWidget::set_pressure (Pressure pressure)
{
	_pressure = pressure;
	update();
}


inline void
EFISWidget::set_pressure_display_hpa (bool hpa)
{
	_pressure_display_hpa = hpa;
	update();
}


inline void
EFISWidget::set_pressure_visible (bool visible)
{
	_pressure_visible = visible;
	update();
}


inline void
EFISWidget::set_standard_pressure (bool standard)
{
	_standard_pressure = standard;
	update();
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


inline void
EFISWidget::set_cmd_altitude (Feet feet)
{
	_cmd_altitude = feet;
	update();
}


inline void
EFISWidget::set_cmd_altitude_visible (bool visible)
{
	_cmd_altitude_visible = visible;
	update();
}


inline void
EFISWidget::set_cmd_climb_rate (FeetPerMinute fpm)
{
	_cmd_climb_rate = fpm;
	update();
}


inline void
EFISWidget::set_cmd_climb_rate_visible (bool visible)
{
	_cmd_climb_rate_visible = visible;
	update();
}


inline void
EFISWidget::set_cmd_speed (Knots knots)
{
	_cmd_speed = knots;
	update();
}


inline void
EFISWidget::set_cmd_speed_visible (bool visible)
{
	_cmd_speed_visible = visible;
	update();
}


inline void
EFISWidget::set_flight_director_pitch (Angle pitch)
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


inline void
EFISWidget::set_flight_director_roll (Angle roll)
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
EFISWidget::set_control_stick_pitch (Angle pitch)
{
	_control_stick_pitch = pitch;
	update();
}


inline void
EFISWidget::set_control_stick_roll (Angle roll)
{
	_control_stick_roll = roll;
	update();
}


inline void
EFISWidget::set_control_stick_visible (bool visible)
{
	_control_stick_visible = visible;
	update();
}


inline void
EFISWidget::set_approach_reference_visible (bool visible)
{
	_approach_reference_visible = visible;
	update();
}


inline void
EFISWidget::set_vertical_deviation (Angle deviation)
{
	_vertical_deviation_deg = deviation;
	update();
}


inline void
EFISWidget::set_vertical_deviation_visible (bool visible)
{
	_vertical_deviation_visible = visible;
	update();
}


inline void
EFISWidget::set_lateral_deviation (Angle deviation)
{
	_lateral_deviation_deg = deviation;
	update();
}


inline void
EFISWidget::set_lateral_deviation_visible (bool visible)
{
	_lateral_deviation_visible = visible;
	update();
}


inline void
EFISWidget::set_runway_visible (bool visible)
{
	_runway_visible = visible;
	update();
}


inline void
EFISWidget::set_runway_position (Angle position)
{
	_runway_position = position;
	update();
}


inline void
EFISWidget::set_approach_hint (QString hint)
{
	_approach_hint = hint;
	update();
}


inline void
EFISWidget::set_dme_distance (Length distance)
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


inline void
EFISWidget::set_localizer_id (QString const& loc_id)
{
	_localizer_id = loc_id;
	update();
}


inline void
EFISWidget::set_localizer_magnetic_bearing (Angle mag_bearing)
{
	_localizer_magnetic_bearing = mag_bearing;
	update();
}


inline void
EFISWidget::set_localizer_info_visible (bool visible)
{
	_localizer_info_visible = visible;
	update();
}


inline void
EFISWidget::set_control_hint (QString const& hint)
{
	if (_control_hint != hint)
		_control_hint_ts = QDateTime::currentDateTime();
	_control_hint = hint;
	update();
}


inline void
EFISWidget::set_control_hint_visible (bool visible)
{
	if (_control_hint_visible != visible)
		_control_hint_ts = QDateTime::currentDateTime();
	_control_hint_visible = visible;
	update();
}


inline void
EFISWidget::set_fma_visible (bool visible)
{
	_fma_visible = visible;
	update();
}


inline void
EFISWidget::set_fma_speed_hint (QString const& hint)
{
	if (_fma_speed_hint != hint)
		_fma_speed_ts = QDateTime::currentDateTime();
	_fma_speed_hint = hint;
	update();
}


inline void
EFISWidget::set_fma_speed_small_hint (QString const& hint)
{
	if (_fma_speed_small_hint != hint)
		_fma_speed_small_ts = QDateTime::currentDateTime();
	_fma_speed_small_hint = hint;
	update();
}


inline void
EFISWidget::set_fma_lateral_hint (QString const& hint)
{
	if (_fma_lateral_hint != hint)
		_fma_lateral_ts = QDateTime::currentDateTime();
	_fma_lateral_hint = hint;
	update();
}


inline void
EFISWidget::set_fma_lateral_small_hint (QString const& hint)
{
	if (_fma_lateral_small_hint != hint)
		_fma_lateral_small_ts = QDateTime::currentDateTime();
	_fma_lateral_small_hint = hint;
	update();
}


inline void
EFISWidget::set_fma_vertical_hint (QString const& hint)
{
	if (_fma_vertical_hint != hint)
		_fma_vertical_ts = QDateTime::currentDateTime();
	_fma_vertical_hint = hint;
	update();
}


inline void
EFISWidget::set_fma_vertical_small_hint (QString const& hint)
{
	if (_fma_vertical_small_hint != hint)
		_fma_vertical_small_ts = QDateTime::currentDateTime();
	_fma_vertical_small_hint = hint;
	update();
}


inline void
EFISWidget::set_fov (Angle degrees)
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
EFISWidget::blink_speed()
{
	_speed_blink = !_speed_blink;
}


inline void
EFISWidget::blink_baro()
{
	_baro_blink = !_baro_blink;
}


inline float
EFISWidget::pitch_to_px (Angle degrees) const
{
	float const correction = 0.775f;
	return -degrees / (_fov * correction) * wh();
}


inline float
EFISWidget::heading_to_px (Angle degrees) const
{
	return pitch_to_px (-degrees);
}


inline QColor
EFISWidget::get_baro_color() const
{
	if (_baro_blinking_warning->isActive())
		return _warning_color_2;
	return _navigation_color;
}


inline bool
EFISWidget::is_newly_set (QDateTime const& timestamp) const
{
	return timestamp.secsTo (_current_datetime) < 10.0;
}

#endif

