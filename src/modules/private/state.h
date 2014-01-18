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
#include <vector>

// Qt:
#include <QtCore/QString>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/utility/rotary_encoder.h>
#include <xefis/utility/range.h>


class Action
{
  public:
	virtual void
	data_updated() = 0;
};


class Button: public Action
{
  public:
	typedef std::function<void()> Callback;

  public:
	// Ctor
	Button (std::string const& button_path, Callback callback):
		_callback (callback)
	{
		_button.set_path (button_path);
	}

	// Action API
	void
	data_updated() override
	{
		if (_button.fresh() && *_button)
			_callback();
	}

  protected:
	Xefis::PropertyBoolean	_button;
	Callback				_callback;
};


class ToggleButton: public Action
{
  public:
	typedef std::function<void (bool)> Callback;

  public:
	// Ctor
	ToggleButton (std::string const& button_path, std::string const& toggle_path)
	{
		_button.set_path (button_path);
		_toggle.set_path (toggle_path);
	}

	/**
	 * Set press callback.
	 */
	void
	set_callback (Callback callback)
	{
		_callback = callback;
	}

	/**
	 * Set to mode that disallows resetting state
	 * by another press.
	 */
	void
	set_radio_mode()
	{
		_radio_mode = true;
	}

	/**
	 * Return true if button is pressed.
	 */
	bool
	pressed() const
	{
		return _button.read (false);
	}

	/**
	 * Set toggle to false.
	 */
	void
	reset()
	{
		_toggle.write (false);
		call();
	}

	/**
	 * Return LED state.
	 */
	bool
	active() const
	{
		return _toggle.read (false);
	}

	/**
	 * Select this option.
	 */
	void
	select()
	{
		if (_radio_mode)
			_toggle.write (true);
		else
			_toggle.write (!_toggle.read (false));
		call();
	}

	/**
	 * Call the callback.
	 */
	void
	call()
	{
		if (_callback)
			_callback (_toggle.read (false));
	}

	// Action API
	void
	data_updated() override
	{
		if (_button.fresh() && *_button)
			select();
	}

  protected:
	Xefis::PropertyBoolean	_button;
	Xefis::PropertyBoolean	_toggle;
	Callback				_callback;
	bool					_radio_mode = false;
};


class ButtonOptions: public Action
{
  public:
	class Option
	{
	  public:
		// Ctor
		Option (std::string const& button_path, std::string const& toggle_path, int value, bool is_default = false):
			button (button_path, toggle_path),
			value (value),
			is_default (is_default)
		{
			button.set_radio_mode();
		}

		ToggleButton	button;
		int				value;
		bool			is_default;
	};

	typedef std::vector<Option> Options;

  public:
	// Ctor
	ButtonOptions (std::string const& value_path, Options const& options):
		_options (options)
	{
		_value_target.set_path (value_path);

		// Press the "default" button:
		for (Option& option: _options)
			if (option.is_default)
				option.button.select();
	}

	// Action API
	void
	data_updated() override
	{
		for (Option& option: _options)
			option.button.data_updated();

		for (Option& option: _options)
		{
			if (option.button.pressed())
			{
				for (Option& option_to_reset: _options)
					if (&option_to_reset != &option)
						option_to_reset.button.reset();
				_value_target.write (option.value);
				break;
			}
		}
	}

  private:
	Options					_options;
	Xefis::PropertyInteger	_value_target;
};


class State: public Xefis::Module
{
	static constexpr Length					MinimumsBaroStep		= 10_ft;
	static constexpr Length					MinimumsRadioStep		= 1_ft;
	static constexpr Xefis::Range<Length>	MinimumsBaroRange		= { 0_ft, 5000_ft };
	static constexpr Xefis::Range<Length>	MinimumsRadioRange		= { 0_ft, 20_ft };
	static constexpr Pressure				QNHhPaStep				= 1_hPa;
	static constexpr Pressure				QNHinHgStep				= 0.01_inHg;
	static constexpr Xefis::Range<Pressure>	QNHRange				= { 800_hPa, 1100_hPa };

	enum class MinimumsType
	{
		Baro,
		Radio,
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
	 * Compute course settings.
	 */
	void
	solve_course();

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
	bool								_course_visible				= false;
	// Logic:
	Unique<Button>						_efis_mins_mode_button;
	Unique<ButtonOptions>				_navaid_select_panel;
	Unique<ButtonOptions>				_navaid_left_panel;
	Unique<ButtonOptions>				_navaid_right_panel;
	Unique<ToggleButton>				_afcs_ap_button;
	Unique<ButtonOptions>				_mfd_panel;
	// Buttons, switches, knobs:
	Xefis::PropertyBoolean				_mcp_mins_a;
	Xefis::PropertyBoolean				_mcp_mins_b;
	Unique<Xefis::RotaryEncoder>		_mcp_mins_decoder;
	Observable<Xefis::PropertyBoolean>	_mcp_att;
	Observable<Xefis::PropertyBoolean>	_mcp_appr;
	Observable<Xefis::PropertyBoolean>	_mcp_fd;
	Observable<Xefis::PropertyBoolean>	_mcp_htrk;
	Xefis::PropertyBoolean				_mcp_qnh_a;
	Xefis::PropertyBoolean				_mcp_qnh_b;
	Unique<Xefis::RotaryEncoder>		_mcp_qnh_decoder;
	Observable<Xefis::PropertyBoolean>	_mcp_qnh_hpa;
	Observable<Xefis::PropertyBoolean>	_mcp_std;
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
	// LEDs, displays:
	Xefis::PropertyInteger				_mcp_course_display;
	// Controlled properties:
	Xefis::PropertyBoolean				_setting_efis_fpv_visible;
	Xefis::PropertyBoolean				_setting_efis_show_metric;
	Xefis::PropertyBoolean				_setting_efis_fd_visible;
	Xefis::PropertyBoolean				_setting_efis_appr_visible;
	Xefis::PropertyPressure				_setting_pressure_qnh;
	Xefis::PropertyBoolean				_setting_pressure_display_hpa;
	Xefis::PropertyBoolean				_setting_pressure_use_std;
	Xefis::PropertyLength				_setting_minimums_amsl;
	Xefis::PropertyLength				_setting_minimums_setting;
	Xefis::PropertyString				_setting_minimums_type;
	Xefis::PropertyBoolean				_setting_hsi_display_true_heading;
	Xefis::PropertyBoolean				_setting_hsi_center_on_track;
	Xefis::PropertyInteger				_setting_hsi_display_mode_mfd;
	Xefis::PropertyLength				_setting_hsi_range;
	Xefis::PropertyBoolean				_setting_hsi_home_track_visible;
	Xefis::PropertyAngle				_setting_course;
	Xefis::PropertyBoolean				_setting_course_visible;
	// Other:
	std::vector<ObservableBase*>		_observables;
	std::vector<Xefis::RotaryEncoder*>	_rotary_decoders;
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

