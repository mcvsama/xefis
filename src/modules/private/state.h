/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
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
#include <xefis/core/v1/property.h>
#include <xefis/utility/actions.h>
#include <xefis/utility/delta_decoder.h>
#include <xefis/utility/range.h>


class State: public xf::Module
{
	static constexpr Length					MinimumsBaroStep		= 10_ft;
	static constexpr Length					MinimumsRadioStep		= 1_ft;
	static constexpr xf::Range<Length>		MinimumsBaroRange		= { 0_ft, 5000_ft };
	static constexpr xf::Range<Length>		MinimumsRadioRange		= { 0_ft, 20_ft };
	static constexpr Pressure				QNHhPaStep				= 1_hPa;
	static constexpr Pressure				QNHinHgStep				= 0.01_inHg;
	static constexpr xf::Range<Pressure>	QNHRange				= { 800_hPa, 1100_hPa };

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
		virtual
		~ObservableBase();

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
			Observable (xf::PropertyPath const& path);

			// Ctor
			Observable (xf::PropertyPath const& path, Callback callback);

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
			set_path (xf::PropertyPath const& path);

			/**
			 * Set callback.
			 */
			void
			set_callback (Callback callback);

			/**
			 * Set property's path and callback.
			 */
			void
			observe (xf::PropertyPath const& path, Callback callback);

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
	State (xf::ModuleManager* module_manager, QDomElement const& config);

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
	make_switch (Observable<xf::PropertyBoolean>& bool_observable, std::function<void()>);

	/**
	 * Set observable to toggle given target_switch.
	 */
	void
	make_toggle (Observable<xf::PropertyBoolean>& bool_observable, xf::PropertyBoolean& target_switch);

	/**
	 * Set observable to write given integer to target_property.
	 */
	void
	make_int_writer (Observable<xf::PropertyBoolean>& bool_observable, xf::PropertyInteger& target_property, int value);

  private:
	MinimumsType					_minimums_type				= MinimumsType::Baro;
	Length							_minimums_setting_baro		= 0_ft;
	Length							_minimums_setting_radio		= 0_ft;
	Pressure						_qnh_setting				= 29.92_inHg;
	Angle							_course						= 0_deg;
	bool							_course_visible				= false;
	// Logic:
	Unique<xf::ButtonAction>		_efis_mins_mode_button;
	Unique<xf::ButtonOptionsAction>	_navaid_select_panel;
	Unique<xf::ButtonOptionsAction>	_navaid_left_panel;
	Unique<xf::ButtonOptionsAction>	_navaid_right_panel;
	Unique<xf::ToggleButtonAction>	_afcs_ap_button;
	Unique<xf::ButtonOptionsAction>	_mfd_panel;
	// Buttons, switches, knobs:
	xf::PropertyInteger				_mcp_mins_value;
	Unique<xf::DeltaDecoder>		_mcp_mins_decoder;
	Observable<xf::PropertyBoolean>	_mcp_att;
	Observable<xf::PropertyBoolean>	_mcp_appr;
	Observable<xf::PropertyBoolean>	_mcp_fd;
	Observable<xf::PropertyBoolean>	_mcp_htrk;
	xf::PropertyInteger				_mcp_qnh_value;
	Unique<xf::DeltaDecoder>		_mcp_qnh_decoder;
	Observable<xf::PropertyBoolean>	_mcp_qnh_hpa;
	Observable<xf::PropertyBoolean>	_mcp_std;
	Observable<xf::PropertyBoolean>	_mcp_metric;
	Observable<xf::PropertyBoolean>	_mcp_fpv;
	xf::PropertyInteger				_mcp_range_value;
	Unique<xf::DeltaDecoder>		_mcp_range_decoder;
	Observable<xf::PropertyBoolean>	_mcp_range_ctr;
	Observable<xf::PropertyBoolean>	_mcp_hdg_trk;
	Observable<xf::PropertyBoolean>	_mcp_mag_tru;
	xf::PropertyInteger				_mcp_course_value;
	Unique<xf::DeltaDecoder>		_mcp_course_decoder;
	Observable<xf::PropertyBoolean>	_mcp_course_hide;
	// LEDs, displays:
	xf::PropertyInteger				_mcp_course_display;
	// Controlled properties:
	xf::PropertyBoolean				_setting_efis_fpv_visible;
	xf::PropertyBoolean				_setting_efis_show_metric;
	xf::PropertyBoolean				_setting_efis_fd_visible;
	xf::PropertyBoolean				_setting_efis_appr_visible;
	xf::PropertyPressure			_setting_pressure_qnh;
	xf::PropertyBoolean				_setting_pressure_display_hpa;
	xf::PropertyBoolean				_setting_pressure_use_std;
	xf::PropertyLength				_setting_minimums_amsl;
	xf::PropertyLength				_setting_minimums_setting;
	xf::PropertyString				_setting_minimums_type;
	xf::PropertyBoolean				_setting_hsi_display_true_heading;
	xf::PropertyBoolean				_setting_hsi_center_on_track;
	xf::PropertyInteger				_setting_hsi_display_mode_mfd;
	xf::PropertyLength				_setting_hsi_range;
	xf::PropertyBoolean				_setting_hsi_home_track_visible;
	xf::PropertyAngle				_setting_course;
	xf::PropertyBoolean				_setting_course_visible;
	// Other:
	std::vector<ObservableBase*>	_observables;
	std::vector<xf::DeltaDecoder*>	_rotary_decoders;
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
	State::Observable<P>::Observable (xf::PropertyPath const& path):
		_property (path)
	{
		reset();
	}


template<class P>
	inline
	State::Observable<P>::Observable (xf::PropertyPath const& path, Callback callback):
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
	State::Observable<P>::set_path (xf::PropertyPath const& path)
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
	State::Observable<P>::observe (xf::PropertyPath const& path, Callback callback)
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

