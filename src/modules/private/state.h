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

#ifndef XEFIS__MODULES__PRIVATE__STATE_H__INCLUDED
#define XEFIS__MODULES__PRIVATE__STATE_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>

// Qt:
#include <QtCore/QString>
#include <QtCore/QTime>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/utility/rotary_encoder.h>
#include <xefis/utility/range.h>


class State:
	public QObject,
	public Xefis::Module
{
	Q_OBJECT

	static constexpr Length					MinimumsBaroStep		= 10_ft;
	static constexpr Length					MinimumsRadioStep		= 1_ft;
	static constexpr Xefis::Range<Length>	MinimumsBaroRange		= { 0_ft, 5000_ft };
	static constexpr Xefis::Range<Length>	MinimumsRadioRange		= { 0_ft, 20_ft };
	static constexpr Pressure				QNHhPaStep				= 1_hPa;
	static constexpr Pressure				QNHinHgStep				= 0.01_inHg;
	static constexpr Xefis::Range<Pressure>	QNHRange				= { 800_hPa, 1100_hPa };
	static constexpr Xefis::Range<Speed>	CmdSpeedRange			= { 10_kt, 300_kt };
	static constexpr Xefis::Range<Length>	CmdAltitudeRange		= { -5000_ft, 50000_ft };
	static constexpr Speed					CmdVSpdStep				= 10_fpm;
	static constexpr Xefis::Range<Speed>	CmdVSpdRange			= { -6000_fpm, 6000_fpm };
	static constexpr Angle					HeadingHoldPitchLimit	= 30_deg;
	static constexpr Angle					AltitudeHoldRollLimit	= 30_deg;

	enum class MinimumsType
	{
		Baro,
		Radio,
	};

	enum class MFDMode: int
	{
		EICAS	= 0,
		ND		= 1,
		CHKLST	= 2,
	};

	enum class CmdAltitudeStep
	{
		Ft10,
		Ft100,
	};

	enum class SpeedMode
	{
		None,
		Select,
		Hold,
	};

	enum class LateralMode
	{
		None,
		LNAV,	// Follow a course provided by flight plan module.
		Select,	// Follow a course set on the counter.
		Hold,	// Follow a course set on the counter.
	};

	enum class VerticalMode
	{
		None,
		VNAV,	// Follow a course provided by flight plan module.
		Select,	// Follow a course set on the counter.
		Hold,	// Follow a course set on the counter.
	};

	/**
	 * Common base for Observable classes.
	 */
	class ObservableBase
	{
	  public:
		// Dtor
		virtual ~ObservableBase();

		/**
		 * Check if the observed value has changed.
		 */
		virtual void
		process() = 0;
	};

	/**
	 * Observable property with callback issued
	 * when value of the property changes.
	 * Encapsulates its own Property object.
	 */
	template<class tProperty>
		class Observable: public ObservableBase
		{
		  public:
			typedef tProperty							PropertyType;
			typedef std::function<void(PropertyType&)>	Callback;

		  public:
			// Ctor
			Observable();

			// Ctor
			Observable (std::string const& path);

			// Ctor
			Observable (std::string const& path, Callback callback);

			/**
			 * Access internal property object.
			 */
			PropertyType&
			property();

			/**
			 * Access internal property object.
			 */
			PropertyType const&
			property() const;

			/**
			 * Set property's path and reset.
			 */
			void
			set_path (std::string const& path);

			/**
			 * Set callback.
			 */
			void
			set_callback (Callback callback);

			/**
			 * Set property's path and callback.
			 */
			void
			observe (std::string const& path, Callback callback);

			// ObservableBase API
			void
			process() override;

			/**
			 * Forget about the change.
			 */
			void
			reset();

		  private:
			PropertyType				_property;
			typename PropertyType::Type	_prev_value;
			Callback					_callback;
		};

	/**
	 * Observable that watches boolean momentary switch,
	 * and switches another boolean property in latching way.
	 */
	class Switch: public Observable<Xefis::PropertyBoolean>
	{
	  public:
		// Ctor
		Switch (std::string const& momentary_path, std::string const& latching_path);

	  private:
		Xefis::PropertyBoolean	_latching_prop;
	};

  public:
	State (Xefis::ModuleManager* module_manager, QDomElement const& config);

	void
	data_updated() override;

  private slots:
	/**
	 * Check for current FD status and switch modes eg. when
	 * selected heading is acquired, or issue a notice when altitude
	 * is about to be acquired, etc.
	 */
	void
	periodic_fd_check();

  private:
	void
	prepare_efis_settings();

	void
	prepare_afcs_main_panel();

	void
	prepare_speed_panel();

	void
	prepare_heading_panel();

	void
	prepare_nav_panel();

	void
	prepare_altitude_panel();

	void
	prepare_vspd_panel();

	void
	prepare_lights_gpws_mfd_panels();

	/**
	 * Compute _setting_minimums_amsl from landing altitude and minimums setting.
	 */
	void
	solve_minimums();

	/**
	 * Compute _qnh_setting.
	 */
	void
	solve_pressure();

	/**
	 * Compute and solve settings of Flight Director.
	 */
	void
	solve_fd();

	/**
	 * Call given callback when button is pressed (property becomes true).
	 */
	void
	make_switch (Observable<Xefis::PropertyBoolean>& bool_observable, std::function<void()>);

	/**
	 * Set observable to toggle given target_switch.
	 */
	void
	make_toggle (Observable<Xefis::PropertyBoolean>& bool_observable, Xefis::PropertyBoolean& target_switch);

	/**
	 * Set observable to write given integer to target_property.
	 */
	void
	make_int_writer (Observable<Xefis::PropertyBoolean>& bool_observable, Xefis::PropertyInteger& target_property, int value);

  private:
	MinimumsType						_minimums_type				= MinimumsType::Baro;
	Length								_minimums_setting_baro		= 0_ft;
	Length								_minimums_setting_radio		= 0_ft;
	Pressure							_qnh_setting				= 29.92_inHg;
	Angle								_course						= 0_deg;
	Speed								_cmd_speed_counter			= CmdSpeedRange.min();
	Speed								_cmd_speed_selected			= CmdSpeedRange.min();
	Angle								_cmd_heading_counter		= 0_deg;
	Angle								_cmd_heading_selected		= 0_deg;
	Length								_cmd_altitude_counter		= 1000_ft;
	Length								_cmd_altitude_selected		= 1000_ft;
	Speed								_cmd_vspd_counter			= 0_fpm;
	Speed								_cmd_vspd_selected			= 0_fpm;
	CmdAltitudeStep						_cmd_altitude_step			= CmdAltitudeStep::Ft10;
	SpeedMode							_speed_mode					= SpeedMode::None;
	LateralMode							_lateral_mode				= LateralMode::None;
	VerticalMode						_vertical_mode				= VerticalMode::None;
	// Buttons, switches, knobs:
	Observable<Xefis::PropertyBoolean>	_saitek_a;
	Observable<Xefis::PropertyBoolean>	_saitek_b;
	Observable<Xefis::PropertyBoolean>	_saitek_c;
	Observable<Xefis::PropertyBoolean>	_saitek_d;
	Observable<Xefis::PropertyBoolean>	_saitek_shift;
	Observable<Xefis::PropertyBoolean>	_saitek_t1;
	Observable<Xefis::PropertyBoolean>	_saitek_t2;
	Observable<Xefis::PropertyBoolean>	_saitek_t3;
	Observable<Xefis::PropertyBoolean>	_saitek_t4;
	Observable<Xefis::PropertyBoolean>	_saitek_t5;
	Observable<Xefis::PropertyBoolean>	_saitek_t6;
	Observable<Xefis::PropertyBoolean>	_saitek_mode_1;
	Observable<Xefis::PropertyBoolean>	_saitek_mode_2;
	Observable<Xefis::PropertyBoolean>	_saitek_mode_3;
	Observable<Xefis::PropertyBoolean>	_saitek_mfd_startstop;
	Observable<Xefis::PropertyBoolean>	_saitek_mfd_reset;
	Observable<Xefis::PropertyBoolean>	_saitek_function_up;
	Observable<Xefis::PropertyBoolean>	_saitek_function_down;
	Observable<Xefis::PropertyBoolean>	_saitek_function_press;
	Observable<Xefis::PropertyBoolean>	_saitek_mfd_select_up;
	Observable<Xefis::PropertyBoolean>	_saitek_mfd_select_down;
	Observable<Xefis::PropertyBoolean>	_saitek_mfd_select_press;
	Xefis::PropertyBoolean				_mcp_mins_a;
	Xefis::PropertyBoolean				_mcp_mins_b;
	Unique<Xefis::RotaryEncoder>		_mcp_mins_decoder;
	Observable<Xefis::PropertyBoolean>	_mcp_mins_mode;
	Observable<Xefis::PropertyBoolean>	_mcp_appr;
	Observable<Xefis::PropertyBoolean>	_mcp_fd;
	Xefis::PropertyBoolean				_mcp_qnh_a;
	Xefis::PropertyBoolean				_mcp_qnh_b;
	Unique<Xefis::RotaryEncoder>		_mcp_qnh_decoder;
	Observable<Xefis::PropertyBoolean>	_mcp_qnh_hpa;
	Observable<Xefis::PropertyBoolean>	_mcp_std;
	Observable<Xefis::PropertyBoolean>	_mcp_stick;
	Observable<Xefis::PropertyBoolean>	_mcp_metric;
	Observable<Xefis::PropertyBoolean>	_mcp_fpv;
	Xefis::PropertyBoolean				_mcp_range_a;
	Xefis::PropertyBoolean				_mcp_range_b;
	Unique<Xefis::RotaryEncoder>		_mcp_range_decoder;
	Observable<Xefis::PropertyBoolean>	_mcp_range_ctr;
	Observable<Xefis::PropertyBoolean>	_mcp_hdg_trk;
	Observable<Xefis::PropertyBoolean>	_mcp_mag_tru;
	Xefis::PropertyBoolean				_mcp_course_a;
	Xefis::PropertyBoolean				_mcp_course_b;
	Unique<Xefis::RotaryEncoder>		_mcp_course_decoder;
	Observable<Xefis::PropertyBoolean>	_mcp_course_hide;
	Observable<Xefis::PropertyBoolean>	_mcp_ap;
	Observable<Xefis::PropertyBoolean>	_mcp_at;
	Observable<Xefis::PropertyBoolean>	_mcp_prot;
	Observable<Xefis::PropertyBoolean>	_mcp_tac;
	Observable<Xefis::PropertyBoolean>	_mcp_att;
	Observable<Xefis::PropertyBoolean>	_mcp_ct;
	Xefis::PropertyBoolean				_mcp_speed_a;
	Xefis::PropertyBoolean				_mcp_speed_b;
	Unique<Xefis::RotaryEncoder>		_mcp_speed_decoder;
	Observable<Xefis::PropertyBoolean>	_mcp_speed_ias_mach;
	Observable<Xefis::PropertyBoolean>	_mcp_speed_sel;
	Observable<Xefis::PropertyBoolean>	_mcp_speed_hold;
	Xefis::PropertyBoolean				_mcp_heading_a;
	Xefis::PropertyBoolean				_mcp_heading_b;
	Unique<Xefis::RotaryEncoder>		_mcp_heading_decoder;
	Observable<Xefis::PropertyBoolean>	_mcp_heading_hdg_trk;
	Observable<Xefis::PropertyBoolean>	_mcp_heading_sel;
	Observable<Xefis::PropertyBoolean>	_mcp_heading_hold;
	Observable<Xefis::PropertyBoolean>	_mcp_vnav;
	Observable<Xefis::PropertyBoolean>	_mcp_lnav;
	Observable<Xefis::PropertyBoolean>	_mcp_app;
	Xefis::PropertyBoolean				_mcp_altitude_a;
	Xefis::PropertyBoolean				_mcp_altitude_b;
	Unique<Xefis::RotaryEncoder>		_mcp_altitude_decoder;
	Observable<Xefis::PropertyBoolean>	_mcp_altitude_stepch;
	Observable<Xefis::PropertyBoolean>	_mcp_altitude_flch;
	Observable<Xefis::PropertyBoolean>	_mcp_altitude_hold;
	Xefis::PropertyBoolean				_mcp_vspd_a;
	Xefis::PropertyBoolean				_mcp_vspd_b;
	Unique<Xefis::RotaryEncoder>		_mcp_vspd_decoder;
	Observable<Xefis::PropertyBoolean>	_mcp_vspd_vs_fpa;
	Observable<Xefis::PropertyBoolean>	_mcp_vspd_sel;
	Observable<Xefis::PropertyBoolean>	_mcp_vspd_clb_con;
	Observable<Xefis::PropertyBoolean>	_mcp_gpws_flap_inh;
	Observable<Xefis::PropertyBoolean>	_mcp_gpws_gear_inh;
	Observable<Xefis::PropertyBoolean>	_mcp_gpws_terr_inh;
	Observable<Xefis::PropertyBoolean>	_mcp_lights_strobe;
	Observable<Xefis::PropertyBoolean>	_mcp_lights_pos;
	Observable<Xefis::PropertyBoolean>	_mcp_lights_ldg;
	Observable<Xefis::PropertyBoolean>	_mcp_eicas_cancel;
	Observable<Xefis::PropertyBoolean>	_mcp_eicas_recall;
	Observable<Xefis::PropertyBoolean>	_mcp_eicas_ok;
	Observable<Xefis::PropertyBoolean>	_mcp_show_nd;
	Observable<Xefis::PropertyBoolean>	_mcp_show_eicas;
	Observable<Xefis::PropertyBoolean>	_mcp_show_chklst;
	// LEDs, displays:
	Xefis::PropertyInteger				_mcp_course_display;
	Xefis::PropertyInteger				_mcp_speed_display;
	Xefis::PropertyInteger				_mcp_heading_display;
	Xefis::PropertyInteger				_mcp_altitude_display;
	Xefis::PropertyInteger				_mcp_vspd_display;
	Xefis::PropertyBoolean				_mcp_speed_sel_led;
	Xefis::PropertyBoolean				_mcp_speed_hold_led;
	Xefis::PropertyBoolean				_mcp_heading_sel_led;
	Xefis::PropertyBoolean				_mcp_heading_hold_led;
	Xefis::PropertyBoolean				_mcp_lnav_led;
	Xefis::PropertyBoolean				_mcp_vnav_led;
	Xefis::PropertyBoolean				_mcp_altitude_flch_led;
	Xefis::PropertyBoolean				_mcp_altitude_hold_led;
	// Input
	Xefis::PropertyAngle				_orientation_pitch;
	Xefis::PropertyAngle				_orientation_roll;
	Xefis::PropertyAngle				_orientation_magnetic_heading;
	Xefis::PropertyAngle				_orientation_magnetic_track;
	Xefis::PropertySpeed				_speed_ias;
	Xefis::PropertyLength				_altitude_amsl;
	// Controlled properties:
	Xefis::PropertyBoolean				_setting_efis_fpv_visible;
	Xefis::PropertyBoolean				_setting_efis_show_metric;
	Xefis::PropertyBoolean				_setting_efis_fd_visible;
	Xefis::PropertyBoolean				_setting_efis_appr_visible;
	Xefis::PropertyBoolean				_setting_efis_stick_visible;
	Xefis::PropertyInteger				_setting_efis_mfd_mode;
	Xefis::PropertyPressure				_setting_pressure_qnh;
	Xefis::PropertyBoolean				_setting_pressure_display_hpa;
	Xefis::PropertyBoolean				_setting_pressure_use_std;
	Xefis::PropertyLength				_setting_minimums_amsl;
	Xefis::PropertyLength				_setting_minimums_setting;
	Xefis::PropertyString				_setting_minimums_type;
	Xefis::PropertySpeed				_setting_cmd_speed;
	Xefis::PropertyAngle				_setting_cmd_heading;
	Xefis::PropertyLength				_setting_cmd_altitude;
	Xefis::PropertySpeed				_setting_cmd_vspd;
	Xefis::PropertyBoolean				_setting_hsi_display_true_heading;
	Xefis::PropertyBoolean				_setting_hsi_center_on_track;
	Xefis::PropertyInteger				_setting_hsi_display_mode_mfd;
	Xefis::PropertyLength				_setting_hsi_range;
	Xefis::PropertyBoolean				_setting_lights_strobe;
	Xefis::PropertyBoolean				_setting_lights_position;
	Xefis::PropertyBoolean				_setting_lights_landing;
	Xefis::PropertyString				_setting_fma_speed;
	Xefis::PropertyString				_setting_fma_lateral;
	Xefis::PropertyString				_setting_fma_vertical;
	// Other:
	std::vector<ObservableBase*>		_observables;
	std::vector<Xefis::RotaryEncoder*>	_rotary_decoders;
	QTimer*								_periodic_timer;
};


inline
State::ObservableBase::~ObservableBase()
{ }


template<class P>
	inline
	State::Observable<P>::Observable()
	{ }


template<class P>
	inline
	State::Observable<P>::Observable (std::string const& path):
		_property (path)
	{
		reset();
	}


template<class P>
	inline
	State::Observable<P>::Observable (std::string const& path, Callback callback):
		_property (path),
		_callback (callback)
	{
		reset();
	}


template<class P>
	inline typename State::Observable<P>::PropertyType&
	State::Observable<P>::property()
	{
		return _property;
	}


template<class P>
	inline typename State::Observable<P>::PropertyType const&
	State::Observable<P>::property() const
	{
		return _property;
	}


template<class P>
	inline void
	State::Observable<P>::set_path (std::string const& path)
	{
		_property.set_path (path);
		reset();
	}


template<class P>
	inline void
	State::Observable<P>::set_callback (Callback callback)
	{
		_callback = callback;
	}


template<class P>
	inline void
	State::Observable<P>::observe (std::string const& path, Callback callback)
	{
		set_path (path);
		set_callback (callback);
	}


template<class P>
	inline void
	State::Observable<P>::reset()
	{
		_prev_value = *_property;
	}

#endif

