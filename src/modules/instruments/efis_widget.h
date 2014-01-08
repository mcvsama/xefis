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

// Boost:
#include <boost/optional.hpp>

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

	class PaintWorkUnit;

  public:
	class Parameters
	{
		friend class EFISWidget;
		friend class PaintWorkUnit;

	  public:
		bool				old_style							= false;
		bool				show_metric							= false;
		Angle				fov									= 120_deg;
		bool				input_alert_visible					= false;
		// Speed
		bool				speed_failure						= false;
		bool				speed_visible						= false;
		Speed				speed								= 0_kt;
		bool				speed_lookahead_visible				= false;
		Speed				speed_lookahead						= 0_kt;
		bool				speed_minimum_visible				= false;
		Speed				speed_minimum						= 0_kt;
		bool				speed_minimum_maneuver_visible		= false;
		Speed				speed_minimum_maneuver				= 0_kt;
		bool				speed_maximum_maneuver_visible		= false;
		Speed				speed_maximum_maneuver				= 0_kt;
		bool				speed_maximum_visible				= false;
		Speed				speed_maximum						= 0_kt;
		bool				speed_mach_visible					= false;
		float				speed_mach							= 0.f;
		SpeedBugs			speed_bugs;
		// Orientation
		bool				orientation_failure					= false;
		bool				orientation_pitch_visible			= false;
		Angle				orientation_pitch					= 0_deg;
		bool				orientation_roll_visible			= false;
		Angle				orientation_roll					= 0_deg;
		bool				orientation_heading_visible			= false;
		Angle				orientation_heading					= 0_deg;
		bool				orientation_heading_numbers_visible	= false;
		// Slip-skid
		bool				slip_skid_visible					= false;
		float				slip_skid							= 0.f;
		// Flight path vector
		bool				flight_path_marker_failure			= false;
		bool				flight_path_visible					= false;
		Angle				flight_path_alpha					= 0_deg;
		Angle				flight_path_beta					= 0_deg;
		// AOA limit
		bool				critical_aoa_visible				= false;
		Angle				critical_aoa						= 0_deg;
		Angle				aoa_alpha							= 0_deg;
		// Altitude
		bool				altitude_failure					= false;
		bool				altitude_visible					= false;
		Length				altitude							= 0_ft;
		bool				altitude_lookahead_visible			= false;
		Length				altitude_lookahead					= 0_ft;
		bool				altitude_agl_failure				= false;
		bool				altitude_agl_visible				= false;
		Length				altitude_agl						= 0_ft;
		bool				altitude_landing_visible			= false;
		Length				altitude_landing_amsl				= 0_ft;
		Length				altitude_landing_warning_hi			= 0_ft;
		Length				altitude_landing_warning_lo			= 0_ft;
		AltitudeBugs		altitude_bugs;
		// Minimums
		bool				minimums_altitude_visible			= false;
		QString				minimums_type;
		Length				minimums_amsl						= 0_ft;
		Length				minimums_setting					= 0_ft;
		// Vertical speed
		bool				vertical_speed_failure				= false;
		bool				vertical_speed_visible				= false;
		Speed				vertical_speed						= 0_fpm;
		bool				variometer_visible					= false;
		Speed				variometer_rate						= 0_fpm;
		// Pressure settings
		bool				pressure_visible					= false;
		Pressure			pressure_qnh						= 0_inHg;
		bool				pressure_display_hpa				= false;
		bool				use_standard_pressure				= false;
		// Command settings
		bool				cmd_altitude_visible				= false;
		Length				cmd_altitude						= 0_ft;
		bool				cmd_altitude_acquired				= false;
		bool				cmd_vertical_speed_visible			= false;
		Speed				cmd_vertical_speed					= 0_fpm;
		bool				cmd_speed_visible					= false;
		Speed				cmd_speed							= 0_kt;
		// Flight director
		bool				flight_director_failure				= false;
		bool				flight_director_pitch_visible		= false;
		Angle				flight_director_pitch				= 0_deg;
		bool				flight_director_roll_visible		= false;
		Angle				flight_director_roll				= 0_deg;
		// Control stick
		bool				control_stick_visible				= false;
		Angle				control_stick_pitch					= 0_deg;
		Angle				control_stick_roll					= 0_deg;
		// Approach reference
		bool				approach_reference_visible			= false;
		QString				approach_hint;
		bool				dme_distance_visible				= false;
		Length				dme_distance						= 0_nm;
		bool				localizer_info_visible				= false;
		QString				localizer_id;
		Angle				localizer_magnetic_bearing			= 0_deg;
		// Approach, flight path deviations
		bool				deviation_vertical_failure			= false;
		Optional<Angle>		deviation_vertical_approach;
		Optional<Angle>		deviation_vertical_flight_path;
		bool				deviation_lateral_failure			= false;
		Optional<Angle>		deviation_lateral_approach;
		Optional<Angle>		deviation_lateral_flight_path;
		bool				deviation_mixed_mode				= false;
		// Raising runway
		bool				runway_visible						= false;
		Angle				runway_position						= 0_deg;
		// Control hint
		bool				control_hint_visible				= false;
		QString				control_hint;
		// FMA
		bool				fma_visible							= false;
		QString				fma_speed_hint;
		QString				fma_speed_small_hint;
		QString				fma_lateral_hint;
		QString				fma_lateral_small_hint;
		QString				fma_vertical_hint;
		QString				fma_vertical_small_hint;
		// TCAS
		Optional<Angle>		tcas_ra_pitch_minimum;
		Optional<Angle>		tcas_ra_pitch_maximum;
		Optional<Speed>		tcas_ra_vertical_speed_minimum;
		Optional<Speed>		tcas_ra_vertical_speed_maximum;
		// Warning flags
		bool				novspd_flag							= false;
		bool				ldgalt_flag							= false;
		bool				pitch_disagree						= false;
		bool				roll_disagree						= false;
		bool				ias_disagree						= false;
		bool				altitude_disagree					= false;
		bool				roll_warning						= false;
		bool				slip_skid_warning					= false;
		// Speed ladder
		Speed				sl_extent							= 124_kt;
		int					sl_minimum							= 0;
		int					sl_maximum							= 9999;
		int					sl_line_every						= 10;
		int					sl_number_every						= 20;
		// Altitude ladder
		Length				al_extent							= 825_ft;
		int					al_emphasis_every					= 1000;
		int					al_bold_every						= 500;
		int					al_number_every						= 200;
		int					al_line_every						= 100;

	  private:
		/**
		 * Sanitize all parameters.
		 */
		void
		sanitize();
	};

  private:
	class LocalParameters
	{
	  public:
		bool					speed_blink						= false;
		bool					speed_blinking_active			= false;
		bool					minimums_blink					= false;
		bool					minimums_blinking_active		= false;
		QDateTime				altitude_agl_ts					= QDateTime::fromTime_t (0);
		QDateTime				minimums_altitude_ts			= QDateTime::fromTime_t (0);
		QDateTime				control_hint_ts					= QDateTime::fromTime_t (0);
		QDateTime				fma_speed_ts					= QDateTime::fromTime_t (0);
		QDateTime				fma_speed_small_ts				= QDateTime::fromTime_t (0);
		QDateTime				fma_lateral_ts					= QDateTime::fromTime_t (0);
		QDateTime				fma_lateral_small_ts			= QDateTime::fromTime_t (0);
		QDateTime				fma_vertical_ts					= QDateTime::fromTime_t (0);
		QDateTime				fma_vertical_small_ts			= QDateTime::fromTime_t (0);
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
		adi_paint (Xefis::Painter&);

		void
		adi_clear (Xefis::Painter&);

		void
		adi_paint_horizon (Xefis::Painter&);

		void
		adi_paint_pitch (Xefis::Painter&);

		void
		adi_paint_heading (Xefis::Painter&);

		void
		adi_paint_tcas_ra (Xefis::Painter&);

		void
		adi_paint_roll (Xefis::Painter&);

		void
		adi_paint_pitch_disagree (Xefis::Painter&);

		void
		adi_paint_roll_disagree (Xefis::Painter&);

		void
		adi_paint_flight_path_marker (Xefis::Painter&);

		/*
		 * Speed ladder
		 */

		void
		sl_post_resize();

		void
		sl_pre_paint();

		void
		sl_paint (Xefis::Painter&);

		void
		sl_paint_black_box (Xefis::Painter&, float x);

		void
		sl_paint_ias_disagree (Xefis::Painter&, float x);

		void
		sl_paint_ladder_scale (Xefis::Painter&, float x);

		void
		sl_paint_speed_limits (Xefis::Painter&, float x);

		void
		sl_paint_speed_tendency (Xefis::Painter&, float x);

		void
		sl_paint_bugs (Xefis::Painter&, float x);

		void
		sl_paint_mach_number (Xefis::Painter&, float x);

		void
		sl_paint_ap_setting (Xefis::Painter&);

		void
		sl_paint_novspd_flag (Xefis::Painter&);

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
		al_paint (Xefis::Painter&);

		void
		al_paint_black_box (Xefis::Painter&, float x);

		void
		al_paint_altitude_disagree (Xefis::Painter&, float x);

		void
		al_paint_ladder_scale (Xefis::Painter&, float x);

		void
		al_paint_altitude_tendency (Xefis::Painter&, float x);

		void
		al_paint_bugs (Xefis::Painter&, float x);

		void
		al_paint_vertical_speed (Xefis::Painter&, float x);

		void
		al_paint_pressure (Xefis::Painter&, float x);

		void
		al_paint_ap_setting (Xefis::Painter&);

		void
		al_paint_ldgalt_flag (Xefis::Painter&, float x);

		float
		ft_to_px (Length) const;

		float
		scale_vertical_speed (Speed vertical_speed, float max_value = 1.f) const;

		/*
		 * Other
		 */

		void
		paint_center_cross (Xefis::Painter&, bool center_box, bool rest);

		void
		paint_flight_director (Xefis::Painter&);

		void
		paint_control_stick (Xefis::Painter&);

		void
		paint_altitude_agl (Xefis::Painter&);

		void
		paint_minimums_setting (Xefis::Painter&);

		void
		paint_nav (Xefis::Painter&);

		void
		paint_hints (Xefis::Painter&);

		void
		paint_critical_aoa (Xefis::Painter&);

		void
		paint_input_alert (Xefis::Painter&);

		void
		paint_dashed_zone (Xefis::Painter&, QColor const&, QRectF const& target);

		/**
		 * Various failure flags.
		 */

		void
		adi_paint_attitude_failure (Xefis::Painter& painter);

		void
		adi_paint_flight_path_marker_failure (Xefis::Painter&);

		void
		adi_paint_flight_director_falure (Xefis::Painter&);

		void
		sl_paint_failure (Xefis::Painter&);

		void
		al_paint_vertical_speed_failure (Xefis::Painter&, float x);

		void
		al_paint_failure (Xefis::Painter&);

		void
		paint_radar_altimeter_failure (Xefis::Painter&);

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
		paint_rotating_value (Xefis::Painter& painter,
							  QRectF const& rect, float position, float height_scale,
							  QString const& next, QString const& curr, QString const& prev);

		/**
		 * \param	two_zeros
		 *			Two separate zeros, for positive and negative values.
		 * \param	zero_mark
		 *			Draw red/green/blank mark instead of zero.
		 */
		void
		paint_rotating_digit (Xefis::Painter& painter,
							  QRectF const& box, float value, int round_target, float const height_scale, float const delta, float const phase,
							  bool two_zeros, bool zero_mark, bool black_zero = false);

		/**
		 * Paint horizontal failure flag.
		 */
		void
		paint_horizontal_failure_flag (Xefis::Painter& painter, QPointF const& center, float pixel_font_size, QString const& message);

		/**
		 * Paint vertical failure flag.
		 */
		void
		paint_vertical_failure_flag (Xefis::Painter& painter, QPointF const& center, float pixel_font_size, QString const& message);

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
		LocalParameters		_locals;
		LocalParameters		_locals_next;
		float				_max_w_h;
		float				_q;
		QColor				_sky_color;
		QColor				_sky_shadow;
		QColor				_ground_color;
		QColor				_ground_shadow;
		QColor				_ladder_color;
		QColor				_ladder_border_color;
		QTransform			_center_transform;
		QTransform			_pitch_transform;
		QTransform			_roll_transform;
		QTransform			_heading_transform;
		QTransform			_horizon_transform;
		QDateTime			_current_datetime;

		/*
		 * ADI
		 */

		QRectF				_adi_sky_rect;
		QRectF				_adi_gnd_rect;
		QPainterPath		_flight_path_marker_shape;
		QPointF				_flight_path_marker_position;
		QPainterPath		_old_horizon_clip;
		QPainterPath		_pitch_scale_clipping_path;

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
		QPolygonF			_sl_bug_shape;

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
		QRectF				_al_metric_box_rect;
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
	 * Set new params for the widget.
	 */
	void
	set_params (Parameters const&);

  protected:
	// API of QWidget
	void
	resizeEvent (QResizeEvent*) override;

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
	PaintWorkUnit		_local_paint_work_unit;
	Parameters			_params;
	LocalParameters		_locals;
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
	if (_params.altitude < _params.minimums_amsl)
		return _warning_color_2;
	return _navigation_color;
}


inline bool
EFISWidget::PaintWorkUnit::is_newly_set (QDateTime const& timestamp, Time time) const
{
	return timestamp.secsTo (_current_datetime) < time.s();
}

#endif

