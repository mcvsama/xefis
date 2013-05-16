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
#include <atomic>
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
#include <xefis/utility/painter.h>


class EFISWidget: public Xefis::InstrumentWidget
{
	Q_OBJECT

	typedef std::map<QString, Speed> SpeedBugs;
	typedef std::map<QString, Length> AltitudeBugs;

	class Parameters
	{
	  public:
		Angle				fov								= 120_deg;
		bool				input_alert_visible				= false;
		Angle				pitch							= 0_deg;
		Angle				pitch_limit						= 0_deg;
		bool				pitch_visible					= false;
		bool				pitch_limit_visible				= false;
		Angle				roll							= 0_deg;
		Angle				roll_limit						= 0_deg;
		bool				roll_visible					= false;
		Angle				heading							= 0_deg;
		bool				heading_visible					= false;
		bool				heading_numbers_visible			= false;
		float				slip_skid						= 0.f;
		float				slip_skid_limit					= 0.f;
		bool				slip_skid_visible				= false;
		Angle				flight_path_alpha				= 0_deg;
		Angle				flight_path_beta				= 0_deg;
		bool				flight_path_visible				= false;
		Speed				speed							= 0_kt;
		bool				speed_visible					= false;
		Speed				speed_tendency					= 0_kt;
		bool				speed_tendency_visible			= false;
		bool				novspd_flag						= false;
		Length				altitude						= 0_ft;
		bool				altitude_visible				= false;
		Length				altitude_tendency				= 0_ft;
		bool				altitude_tendency_visible		= false;
		Length				altitude_agl					= 0_ft;
		bool				altitude_agl_visible			= false;
		QDateTime			altitude_agl_ts;
		bool				altitude_warnings_visible		= false;
		Length				minimums_altitude				= 0_ft;
		bool				minimums_altitude_visible		= false;
		QDateTime			minimums_altitude_ts;
		Speed				climb_rate						= 0_fpm;
		bool				climb_rate_visible				= false;
		Speed				variometer_rate					= 0_fpm;
		bool				variometer_visible				= false;
		float				mach							= 0.f;
		bool				mach_visible					= false;
		Pressure			pressure						= 0_inHg;
		bool				pressure_display_hpa			= false;
		bool				pressure_visible				= false;
		bool				use_standard_pressure			= false;
		Speed				minimum_speed					= 0_kt;
		bool				minimum_speed_visible			= false;
		Speed				warning_speed					= 0_kt;
		bool				warning_speed_visible			= false;
		Speed				maximum_speed					= 0_kt;
		bool				maximum_speed_visible			= false;
		Length				cmd_altitude					= 0_ft;
		bool				cmd_altitude_visible			= false;
		Speed				cmd_climb_rate					= 0_fpm;
		bool				cmd_climb_rate_visible			= false;
		Speed				cmd_speed						= 0_kt;
		bool				cmd_speed_visible				= false;
		Angle				flight_director_pitch			= 0_deg;
		bool				flight_director_pitch_visible	= false;
		Angle				flight_director_roll			= 0_deg;
		bool				flight_director_roll_visible	= false;
		Angle				control_stick_pitch				= 0_deg;
		Angle				control_stick_roll				= 0_deg;
		bool				control_stick_visible			= false;
		bool				approach_reference_visible		= false;
		Angle				vertical_deviation_deg			= 0_deg;
		bool				vertical_deviation_visible		= false;
		Angle				lateral_deviation_deg			= 0_deg;
		bool				lateral_deviation_visible		= false;
		bool				runway_visible					= false;
		Angle				runway_position					= 0_deg;
		QString				approach_hint;
		Length				dme_distance					= 0_nm;
		bool				dme_distance_visible			= false;
		QString				localizer_id;
		Angle				localizer_magnetic_bearing		= 0_deg;
		bool				localizer_info_visible			= false;
		QString				control_hint;
		bool				control_hint_visible			= false;
		QDateTime			control_hint_ts;
		bool				fma_visible						= false;
		QString				fma_speed_hint;
		QDateTime			fma_speed_ts;
		QString				fma_speed_small_hint;
		QDateTime			fma_speed_small_ts;
		QString				fma_lateral_hint;
		QDateTime			fma_lateral_ts;
		QString				fma_lateral_small_hint;
		QDateTime			fma_lateral_small_ts;
		QString				fma_vertical_hint;
		QDateTime			fma_vertical_ts;
		QString				fma_vertical_small_hint;
		QDateTime			fma_vertical_small_ts;
		SpeedBugs			speed_bugs;
		AltitudeBugs		altitude_bugs;
		bool				speed_blink						= false;
		bool				speed_blinking_active			= false;
		bool				minimums_blink					= false;
		bool				minimums_blinking_active		= false;

		/*
		 * Speed ladder
		 */

		Speed				sl_extent						= 124_kt;
		int					sl_minimum						= 0;
		int					sl_maximum						= 9999;
		int					sl_line_every					= 10;
		int					sl_number_every					= 20;

		/*
		 * Altitude ladder
		 */

		int					al_line_every					= 100;
		int					al_number_every					= 200;
		int					al_bold_every					= 500;
		Length				al_extent						= 825_ft;
	};

	class PaintWorkUnit:
		public InstrumentWidget::PaintWorkUnit,
		public Xefis::InstrumentAids
	{
		friend class EFISWidget;

	  public:
		PaintWorkUnit (EFISWidget*);

		~PaintWorkUnit() noexcept { }

	  private:
		void
		pop_params() override;

		void
		resized() override;

		void
		paint (QImage&) override;

		/*
		 * ADI
		 */

		void
		adi_post_resize();

		void
		adi_pre_paint();

		void
		adi_paint (Painter&);

		void
		adi_paint_horizon (Painter&);

		void
		adi_paint_pitch (Painter&);

		void
		adi_paint_roll (Painter&);

		void
		adi_paint_heading (Painter&);

		void
		adi_paint_flight_path_marker (Painter&);

		/*
		 * Speed ladder
		 */

		void
		sl_post_resize();

		void
		sl_pre_paint();

		void
		sl_paint (Painter&);

		void
		sl_paint_black_box (Painter&, float x);

		void
		sl_paint_ladder_scale (Painter&, float x);

		void
		sl_paint_speed_limits (Painter&, float x);

		void
		sl_paint_speed_tendency (Painter&, float x);

		void
		sl_paint_bugs (Painter&, float x);

		void
		sl_paint_mach_number (Painter&, float x);

		void
		sl_paint_ap_setting (Painter&);

		void
		sl_paint_novspd (Painter&);

		float
		kt_to_px (Speed) const;

		/*
		 * Altitude ladder
		 */

		void
		al_post_resize();

		void
		al_pre_paint();

		void
		al_paint (Painter&);

		void
		al_paint_black_box (Painter&, float x);

		void
		al_paint_ladder_scale (Painter&, float x);

		void
		al_paint_altitude_tendency (Painter&, float x);

		void
		al_paint_bugs (Painter&, float x);

		void
		al_paint_climb_rate (Painter&, float x);

		void
		al_paint_pressure (Painter&, float x);

		void
		al_paint_ap_setting (Painter&);

		float
		ft_to_px (Length) const;

		float
		scale_cbr (Speed climb_rate) const;

		/*
		 * Other
		 */

		void
		paint_center_cross (Painter&, bool center_box, bool rest);

		void
		paint_flight_director (Painter&);

		void
		paint_control_stick (Painter&);

		void
		paint_altitude_agl (Painter&);

		void
		paint_minimums_setting (Painter&);

		void
		paint_nav (Painter&);

		void
		paint_hints (Painter&);

		void
		paint_pitch_limit (Painter&);

		void
		paint_input_alert (Painter&);

		void
		paint_dashed_zone (Painter&, QColor const&, QRectF const& target);

		/**
		 * Render 'rotatable' value on speed/altitude black box.
		 *
		 * \param	painter
		 *			Painter to use.
		 * \param	position
		 *			Text position, [-0.5, 0.5].
		 * \param	next, curr, prev
		 *			Texts to render. Special value "G" paints green dashed zone, "R" paints red dashed zone.
		 */
		void
		paint_rotating_value (Painter& painter,
							  QRectF const& rect, float position, float height_scale,
							  QString const& next, QString const& curr, QString const& prev);

		/**
		 * \param	two_zeros
		 *			Two separate zeros, for positive and negative values.
		 * \param	zero_mark
		 *			Draw red/green/blank mark instead of zero.
		 */
		void
		paint_rotating_digit (Painter& painter,
							  QRectF const& box, float value, int round_target, float const height_scale, float const delta, float const phase,
							  bool two_zeros, bool zero_mark, bool black_zero = false);

		float
		pitch_to_px (Angle degrees) const;

		float
		heading_to_px (Angle degrees) const;

		QPainterPath
		get_pitch_scale_clipping_path() const;

		QColor
		get_minimums_color() const;

		bool
		is_newly_set (QDateTime const& timestamp, Time time = 10_s) const;

	  private:
		Parameters			_params;
		Parameters			_params_next;
		float				_w;
		float				_h;
		float				_max_w_h;
		float				_q;
		QColor				_sky_color;
		QColor				_sky_shadow;
		QColor				_ground_color;
		QColor				_ground_shadow;
		QColor				_ladder_color;
		QColor				_ladder_border_color;
		QColor				_warning_color_1;
		QColor				_warning_color_2;
		QTransform			_center_transform;
		QTransform			_pitch_transform;
		QTransform			_roll_transform;
		QTransform			_heading_transform;
		QTransform			_horizon_transform;
		TextPainter::Cache	_text_painter_cache;
		QDateTime			_current_datetime;

		/*
		 * ADI
		 */

		QRectF				_adi_sky_rect;
		QRectF				_adi_gnd_rect;
		QPainterPath		_flight_path_marker_shape;
		QPointF				_flight_path_marker_position;

		/*
		 * Speed ladder
		 */

		QTransform			_sl_transform;
		Speed				_sl_min_shown;
		Speed				_sl_max_shown;
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
		Length				_al_min_shown;
		Length				_al_max_shown;
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
	};

  public:
	// Ctor
	EFISWidget (QWidget* parent, Xefis::WorkPerformer*);

	// Dtor
	~EFISWidget();

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
	set_speed (Speed);

	/**
	 * Toggle visibility of the speed scale.
	 */
	void
	set_speed_visible (bool visible);

	/**
	 * Set speed tendency value.
	 */
	void
	set_speed_tendency (Speed);

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
	set_altitude (Length);

	/**
	 * Toggle visibility of the altitude scale.
	 */
	void
	set_altitude_visible (bool visible);

	/**
	 * Set altitude tendency value.
	 */
	void
	set_altitude_tendency (Length);

	/**
	 * Set altitude tendency arrow visibility.
	 */
	void
	set_altitude_tendency_visible (bool visible);

	/**
	 * Set radar altitude.
	 */
	void
	set_altitude_agl (Length);

	/**
	 * Set radar altitude visibility.
	 */
	void
	set_altitude_agl_visible (bool visible);

	/**
	 * Set visibility of the altitude warnings (500 and 1000 ft) above
	 * max of AGL altitude and LDG altitude or 0.
	 */
	void
	set_altitude_warnings_visible (bool visible);

	/**
	 * Set minimums altitude.
	 */
	void
	set_minimums_altitude (Length);

	/**
	 * Set minimums altitude visibility.
	 */
	void
	set_minimums_altitude_visible (bool visible);

	/**
	 * Set climb rate.
	 */
	void
	set_climb_rate (Speed);

	/**
	 * Set climb rate visibility.
	 */
	void
	set_climb_rate_visible (bool visible);

	/**
	 * Set vario rate.
	 */
	void
	set_variometer_rate (Speed);

	/**
	 * Set variometer visibility.
	 */
	void
	set_variometer_visible (bool visible);

	/**
	 * Add new speed bug.
	 */
	void
	add_speed_bug (QString name, Speed);

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
	add_altitude_bug (QString name, Length altitude);

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
	set_minimum_speed (Speed);

	/**
	 * Set minimum speed indicator visibility.
	 */
	void
	set_minimum_speed_visible (bool visible);

	/**
	 * Set warning speed indicator on the speed ladder.
	 */
	void
	set_warning_speed (Speed);

	/**
	 * Set warning speed indicator visibility.
	 */
	void
	set_warning_speed_visible (bool visible);

	/**
	 * Set maximum speed indicator on the speed ladder.
	 */
	void
	set_maximum_speed (Speed);

	/**
	 * Set maximum speed indicator visibility.
	 */
	void
	set_maximum_speed_visible (bool visible);

	/**
	 * Set commanded altitude.
	 */
	void
	set_cmd_altitude (Length);

	/**
	 * Set AP altitude setting visibility.
	 */
	void
	set_cmd_altitude_visible (bool visible);

	/**
	 * Set commanded climb rate setting.
	 */
	void
	set_cmd_climb_rate (Speed);

	/**
	 * Set AP climb rate visibility.
	 */
	void
	set_cmd_climb_rate_visible (bool visible);

	/**
	 * Set autothrottle speed.
	 */
	void
	set_cmd_speed (Speed);

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

  protected:
	// API of InstrumentWidget
	void
	request_repaint() override;

	// API of InstrumentWidget
	void
	push_params() override;

  private:
	/**
	 * Start or stop blinking warning timer on given condition.
	 */
	void
	update_blinker (QTimer* warning_timer, bool condition, bool* blink_state);

  private slots:
	void
	blink_speed();

	void
	blink_minimums();

  private:
	PaintWorkUnit		_paint_work_unit;
	Parameters			_params;
	QTimer*				_speed_blinking_warning		= nullptr;
	QTimer*				_minimums_blinking_warning	= nullptr;
};


inline float
EFISWidget::PaintWorkUnit::kt_to_px (Speed speed) const
{
	return -0.5f * _sl_ladder_rect.height() * (speed - _params.speed) / (0.5 * _params.sl_extent);
}


inline float
EFISWidget::PaintWorkUnit::ft_to_px (Length length) const
{
	return -0.5f * _al_ladder_rect.height() * (length - _params.altitude) / (0.5 * _params.al_extent);
}


inline float
EFISWidget::PaintWorkUnit::pitch_to_px (Angle degrees) const
{
	float const correction = 0.775f;
	return -degrees / (_params.fov * correction) * wh();
}


inline float
EFISWidget::PaintWorkUnit::heading_to_px (Angle degrees) const
{
	return pitch_to_px (-degrees);
}


inline QColor
EFISWidget::PaintWorkUnit::get_minimums_color() const
{
	if (_params.altitude < _params.minimums_altitude)
		return _warning_color_2;
	return _navigation_color;
}


inline bool
EFISWidget::PaintWorkUnit::is_newly_set (QDateTime const& timestamp, Time time) const
{
	return timestamp.secsTo (_current_datetime) < time.s();
}


inline void
EFISWidget::set_speed_ladder_line_every (int knots)
{
	_params.sl_line_every = std::max (1, knots);
	request_repaint();
}


inline void
EFISWidget::set_speed_ladder_number_every (int knots)
{
	_params.sl_number_every = std::max (1, knots);
	request_repaint();
}


inline void
EFISWidget::set_speed_ladder_extent (int knots)
{
	_params.sl_extent = 1_kt * std::max (1, knots);
	request_repaint();
}


inline void
EFISWidget::set_speed_ladder_minimum (int knots)
{
	_params.sl_minimum = std::max (0, knots);
}


inline void
EFISWidget::set_speed_ladder_maximum (int knots)
{
	_params.sl_maximum = std::min (9999, knots);
}


inline void
EFISWidget::set_altitude_ladder_line_every (int feet)
{
	_params.al_line_every = std::max (1, feet);
	request_repaint();
}


inline void
EFISWidget::set_altitude_ladder_number_every (int feet)
{
	_params.al_number_every = std::max (1, feet);
	request_repaint();
}


inline void
EFISWidget::set_altitude_ladder_bold_every (int feet)
{
	_params.al_bold_every = std::max (1, feet);
	request_repaint();
}


inline void
EFISWidget::set_altitude_ladder_extent (int feet)
{
	_params.al_extent = 1_ft * std::max (1, feet);
	request_repaint();
}


inline void
EFISWidget::set_pitch (Angle degrees)
{
	_params.pitch = degrees;
	request_repaint();
}


inline void
EFISWidget::set_pitch_visible (bool visible)
{
	_params.pitch_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_pitch_limit (Angle pitch_limit)
{
	_params.pitch_limit = pitch_limit;
	request_repaint();
}


inline void
EFISWidget::set_pitch_limit_visible (bool visible)
{
	_params.pitch_limit_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_roll (Angle degrees)
{
	_params.roll = degrees;
	request_repaint();
}


inline void
EFISWidget::set_roll_limit (Angle limit)
{
	_params.roll_limit = limit;
	request_repaint();
}


inline void
EFISWidget::set_roll_visible (bool visible)
{
	_params.roll_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_heading (Angle degrees)
{
	_params.heading = degrees;
	request_repaint();
}


inline void
EFISWidget::set_heading_visible (bool visible)
{
	_params.heading_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_heading_numbers_visible (bool visible)
{
	_params.heading_numbers_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_slip_skid (float value)
{
	_params.slip_skid = value;
	request_repaint();
}


inline void
EFISWidget::set_slip_skid_limit (float limit)
{
	_params.slip_skid_limit = limit;
	request_repaint();
}


inline void
EFISWidget::set_slip_skid_visible (bool visible)
{
	_params.slip_skid_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_flight_path_alpha (Angle pitch)
{
	_params.flight_path_alpha = pitch;
	request_repaint();
}


inline void
EFISWidget::set_flight_path_beta (Angle heading)
{
	_params.flight_path_beta = heading;
	request_repaint();
}


inline void
EFISWidget::set_flight_path_marker_visible (bool visible)
{
	_params.flight_path_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_speed (Speed speed)
{
	_params.speed = speed;
	request_repaint();
}


inline void
EFISWidget::set_speed_visible (bool visible)
{
	_params.speed_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_speed_tendency (Speed speed)
{
	_params.speed_tendency = speed;
	request_repaint();
}


inline void
EFISWidget::set_speed_tendency_visible (bool visible)
{
	_params.speed_tendency_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_novspd_flag (bool visible)
{
	_params.novspd_flag = visible;
	request_repaint();
}


inline void
EFISWidget::set_altitude (Length altitude)
{
	Length previous_altitude = _params.altitude;
	_params.altitude = altitude;

	if (previous_altitude > _params.minimums_altitude && altitude < _params.minimums_altitude)
		_params.minimums_altitude_ts = QDateTime::currentDateTime();

	request_repaint();
}


inline void
EFISWidget::set_altitude_visible (bool visible)
{
	_params.altitude_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_altitude_tendency (Length altitude)
{
	_params.altitude_tendency = altitude;
	request_repaint();
}


inline void
EFISWidget::set_altitude_tendency_visible (bool visible)
{
	_params.altitude_tendency_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_altitude_agl (Length altitude)
{
	_params.altitude_agl = altitude;
	request_repaint();
}


inline void
EFISWidget::set_altitude_agl_visible (bool visible)
{
	if (!_params.altitude_agl_visible && visible)
		_params.altitude_agl_ts = QDateTime::currentDateTime();
	_params.altitude_agl_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_altitude_warnings_visible (bool visible)
{
	_params.altitude_warnings_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_minimums_altitude (Length minimums_altitude)
{
	_params.minimums_altitude = minimums_altitude;
	request_repaint();
}


inline void
EFISWidget::set_minimums_altitude_visible (bool visible)
{
	if (_params.minimums_altitude_visible != visible)
		_params.minimums_altitude_ts = QDateTime::currentDateTime();
	_params.minimums_altitude_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_climb_rate (Speed feet_per_minute)
{
	_params.climb_rate = feet_per_minute;
	request_repaint();
}


inline void
EFISWidget::set_climb_rate_visible (bool visible)
{
	_params.climb_rate_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_variometer_rate (Speed feet_per_minute)
{
	_params.variometer_rate = feet_per_minute;
	request_repaint();
}


inline void
EFISWidget::set_variometer_visible (bool visible)
{
	_params.variometer_visible = visible;
	request_repaint();
}


inline void
EFISWidget::add_speed_bug (QString name, Speed speed)
{
	_params.speed_bugs[name] = speed;
	request_repaint();
}


inline void
EFISWidget::remove_speed_bug (QString name)
{
	if (name.isNull())
		_params.speed_bugs.clear();
	else
		_params.speed_bugs.erase (name);
	request_repaint();
}


inline void
EFISWidget::add_altitude_bug (QString name, Length altitude)
{
	_params.altitude_bugs[name] = altitude;
	request_repaint();
}


inline void
EFISWidget::remove_altitude_bug (QString name)
{
	if (name.isNull())
		_params.altitude_bugs.clear();
	else
		_params.altitude_bugs.erase (name);
	request_repaint();
}


inline void
EFISWidget::set_mach (float value)
{
	_params.mach = value;
	request_repaint();
}


inline void
EFISWidget::set_mach_visible (bool visible)
{
	_params.mach_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_pressure (Pressure pressure)
{
	_params.pressure = pressure;
	request_repaint();
}


inline void
EFISWidget::set_pressure_display_hpa (bool hpa)
{
	_params.pressure_display_hpa = hpa;
	request_repaint();
}


inline void
EFISWidget::set_pressure_visible (bool visible)
{
	_params.pressure_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_standard_pressure (bool standard)
{
	_params.use_standard_pressure = standard;
	request_repaint();
}


inline void
EFISWidget::set_minimum_speed (Speed minimum_speed)
{
	_params.minimum_speed = minimum_speed;
	request_repaint();
}


inline void
EFISWidget::set_minimum_speed_visible (bool visible)
{
	_params.minimum_speed_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_warning_speed (Speed warning_speed)
{
	_params.warning_speed = warning_speed;
	request_repaint();
}


inline void
EFISWidget::set_warning_speed_visible (bool visible)
{
	_params.warning_speed_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_maximum_speed (Speed maximum_speed)
{
	_params.maximum_speed = maximum_speed;
	request_repaint();
}


inline void
EFISWidget::set_maximum_speed_visible (bool visible)
{
	_params.maximum_speed_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_cmd_altitude (Length altitude)
{
	_params.cmd_altitude = altitude;
	request_repaint();
}


inline void
EFISWidget::set_cmd_altitude_visible (bool visible)
{
	_params.cmd_altitude_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_cmd_climb_rate (Speed speed)
{
	_params.cmd_climb_rate = speed;
	request_repaint();
}


inline void
EFISWidget::set_cmd_climb_rate_visible (bool visible)
{
	_params.cmd_climb_rate_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_cmd_speed (Speed speed)
{
	_params.cmd_speed = speed;
	request_repaint();
}


inline void
EFISWidget::set_cmd_speed_visible (bool visible)
{
	_params.cmd_speed_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_flight_director_pitch (Angle pitch)
{
	_params.flight_director_pitch = pitch;
	request_repaint();
}


inline void
EFISWidget::set_flight_director_pitch_visible (bool visible)
{
	_params.flight_director_pitch_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_flight_director_roll (Angle roll)
{
	_params.flight_director_roll = roll;
	request_repaint();
}


inline void
EFISWidget::set_flight_director_roll_visible (bool visible)
{
	_params.flight_director_roll_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_control_stick_pitch (Angle pitch)
{
	_params.control_stick_pitch = pitch;
	request_repaint();
}


inline void
EFISWidget::set_control_stick_roll (Angle roll)
{
	_params.control_stick_roll = roll;
	request_repaint();
}


inline void
EFISWidget::set_control_stick_visible (bool visible)
{
	_params.control_stick_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_approach_reference_visible (bool visible)
{
	_params.approach_reference_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_vertical_deviation (Angle deviation)
{
	_params.vertical_deviation_deg = deviation;
	request_repaint();
}


inline void
EFISWidget::set_vertical_deviation_visible (bool visible)
{
	_params.vertical_deviation_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_lateral_deviation (Angle deviation)
{
	_params.lateral_deviation_deg = deviation;
	request_repaint();
}


inline void
EFISWidget::set_lateral_deviation_visible (bool visible)
{
	_params.lateral_deviation_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_runway_visible (bool visible)
{
	_params.runway_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_runway_position (Angle position)
{
	_params.runway_position = position;
	request_repaint();
}


inline void
EFISWidget::set_approach_hint (QString hint)
{
	_params.approach_hint = hint;
	request_repaint();
}


inline void
EFISWidget::set_dme_distance (Length distance)
{
	_params.dme_distance = distance;
	request_repaint();
}


inline void
EFISWidget::set_dme_distance_visible (bool visible)
{
	_params.dme_distance_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_localizer_id (QString const& loc_id)
{
	_params.localizer_id = loc_id;
	request_repaint();
}


inline void
EFISWidget::set_localizer_magnetic_bearing (Angle mag_bearing)
{
	_params.localizer_magnetic_bearing = mag_bearing;
	request_repaint();
}


inline void
EFISWidget::set_localizer_info_visible (bool visible)
{
	_params.localizer_info_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_control_hint (QString const& hint)
{
	if (_params.control_hint != hint)
		_params.control_hint_ts = QDateTime::currentDateTime();
	_params.control_hint = hint;
	request_repaint();
}


inline void
EFISWidget::set_control_hint_visible (bool visible)
{
	if (_params.control_hint_visible != visible)
		_params.control_hint_ts = QDateTime::currentDateTime();
	_params.control_hint_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_fma_visible (bool visible)
{
	_params.fma_visible = visible;
	request_repaint();
}


inline void
EFISWidget::set_fma_speed_hint (QString const& hint)
{
	if (_params.fma_speed_hint != hint)
		_params.fma_speed_ts = QDateTime::currentDateTime();
	_params.fma_speed_hint = hint;
	request_repaint();
}


inline void
EFISWidget::set_fma_speed_small_hint (QString const& hint)
{
	if (_params.fma_speed_small_hint != hint)
		_params.fma_speed_small_ts = QDateTime::currentDateTime();
	_params.fma_speed_small_hint = hint;
	request_repaint();
}


inline void
EFISWidget::set_fma_lateral_hint (QString const& hint)
{
	if (_params.fma_lateral_hint != hint)
		_params.fma_lateral_ts = QDateTime::currentDateTime();
	_params.fma_lateral_hint = hint;
	request_repaint();
}


inline void
EFISWidget::set_fma_lateral_small_hint (QString const& hint)
{
	if (_params.fma_lateral_small_hint != hint)
		_params.fma_lateral_small_ts = QDateTime::currentDateTime();
	_params.fma_lateral_small_hint = hint;
	request_repaint();
}


inline void
EFISWidget::set_fma_vertical_hint (QString const& hint)
{
	if (_params.fma_vertical_hint != hint)
		_params.fma_vertical_ts = QDateTime::currentDateTime();
	_params.fma_vertical_hint = hint;
	request_repaint();
}


inline void
EFISWidget::set_fma_vertical_small_hint (QString const& hint)
{
	if (_params.fma_vertical_small_hint != hint)
		_params.fma_vertical_small_ts = QDateTime::currentDateTime();
	_params.fma_vertical_small_hint = hint;
	request_repaint();
}


inline void
EFISWidget::set_fov (Angle degrees)
{
	_params.fov = degrees;
	request_repaint();
}


inline void
EFISWidget::set_input_alert_visible (bool visible)
{
	_params.input_alert_visible = visible;
}

#endif

