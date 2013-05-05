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
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


class State: public Xefis::Module
{
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
			PropertyType 				_property;
			typename PropertyType::Type	_prev_value;
			Callback					_callback;
		};

  public:
	State (Xefis::ModuleManager* module_manager, QDomElement const& config);

	void
	data_updated() override;

  private:
	Xefis::PropertyBoolean				_pressure_display_hpa;
	Xefis::PropertyBoolean				_use_standard_pressure;
	Xefis::PropertyBoolean				_follow_track;
	Xefis::PropertyBoolean				_use_true_heading;
	Xefis::PropertyBoolean				_approach_mode;
	Xefis::PropertyBoolean				_flight_director_visible;
	Xefis::PropertyBoolean				_flight_director_enabled;
	Xefis::PropertyInteger				_flight_director_vertical_mode;
	Xefis::PropertyInteger				_flight_director_lateral_mode;
	Xefis::PropertyBoolean				_control_hint_visible;
	Xefis::PropertyString				_control_hint_text;
	Xefis::PropertyBoolean				_control_stick_visible;
	Xefis::PropertyInteger				_fly_by_wire_mode;
	Xefis::PropertyLength				_hsi_aux_range;
	Xefis::PropertyLength				_hsi_nav_range;
	Xefis::PropertyInteger				_hsi_nav_mode;

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

	std::vector<ObservableBase*>		_observables;
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

