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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_FD_PITCH_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_FD_PITCH_H__INCLUDED

// Standard:
#include <cstddef>
#include <type_traits>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/pid_control.h>
#include <xefis/utility/smoother.h>

// Local:
#include "afcs_api.h"


/**
 * Computes desired pitch values to follow.
 * Output depends on pitch-mode setting.
 */
	// TODO disengage if outside safe limits, unless autonomous flag is set
	// (autonomous flag tells whether user has still possibility to control the airplane,
	// that is he is in the range of radio communication).
class AFCS_FD_Pitch: public xf::Module
{
  public:
	// Ctor
	AFCS_FD_Pitch (xf::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

	void
	rescue() override;

  private:
	/**
	 * Compute all needed data and write to output properties.
	 */
	void
	compute_pitch();

	/**
	 * Compute result angle from PID and parameters.
	 */
	template<class P>
		Optional<Angle>
		compute_pitch (xf::PIDControl<double>& pid,
					   xf::Property<P> const& cmd_param,
					   xf::Property<P> const& measured_param,
					   xf::Range<double> const& input_range,
					   Time const& update_dt) const;

	/**
	 * Get the numerical value from xf::Property<T>.
	 */
	template<class SIValue>
		static std::enable_if_t<std::is_base_of<SI::Value, SIValue>::value, double>
		retrieve_numeric (SIValue const&);

	/**
	 * Get the numerical value from a basic type.
	 */
	template<class FundamentalValue>
		static std::enable_if_t<!std::is_base_of<SI::Value, FundamentalValue>::value, double>
		retrieve_numeric (FundamentalValue);

	/**
	 * Called when pitch mode property changes.
	 */
	void
	pitch_mode_changed();

	/**
	 * Override the "operative" output depending on "autonomous" flag.
	 */
	void
	check_autonomous();

  private:
	xf::PIDControl<double>::Settings	_ias_pid_settings			= { 1.0, 0.1, 0.0 };
	xf::PIDControl<double>::Settings	_mach_pid_settings			= { 1.0, 0.1, 0.0 };
	xf::PIDControl<double>::Settings	_alt_pid_settings			= { 1.0, 0.1, 0.0 };
	xf::PIDControl<double>::Settings	_vs_pid_settings			= { 1.0, 0.1, 0.0 };
	xf::PIDControl<double>::Settings	_fpa_pid_settings			= { 1.0, 0.1, 0.0 };
	xf::PIDControl<double>				_ias_pid					= { _ias_pid_settings, 0.0 };
	xf::PIDControl<double>				_mach_pid					= { _mach_pid_settings, 0.0 };
	xf::PIDControl<double>				_alt_pid					= { _alt_pid_settings, 0.0 };
	xf::PIDControl<double>				_vs_pid						= { _vs_pid_settings, 0.0 };
	xf::PIDControl<double>				_fpa_pid					= { _fpa_pid_settings, 0.0 };
	xf::Smoother<double>				_output_pitch_smoother		= 2.5_s;
	afcs_api::PitchMode					_pitch_mode					= afcs_api::PitchMode::None;
	// Input:
	xf::PropertyBoolean					_autonomous;
	xf::PropertyAngle					_pitch_limit;
	xf::PropertyInteger					_cmd_pitch_mode;
	xf::PropertySpeed					_cmd_ias;
	xf::PropertyFloat					_cmd_mach;
	xf::PropertyLength					_cmd_alt;
	xf::PropertySpeed					_cmd_vs;
	xf::PropertyAngle					_cmd_fpa;
	xf::PropertySpeed					_measured_ias;
	xf::PropertyFloat					_measured_mach;
	xf::PropertyLength					_measured_alt;
	xf::PropertySpeed					_measured_vs;
	xf::PropertyAngle					_measured_fpa;
	// Output:
	xf::PropertyAngle					_output_pitch;
	xf::PropertyBoolean					_operative;
	// Other:
	xf::PropertyObserver				_pitch_computer;
};

#endif
