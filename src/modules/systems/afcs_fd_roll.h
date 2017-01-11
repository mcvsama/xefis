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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_FD_ROLL_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_FD_ROLL_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/pid_control.h>
#include <xefis/utility/range_smoother.h>

// Local:
#include "afcs_api.h"


/**
 * Computes desired roll values to follow.
 * Output depends on roll-mode setting.
 */
	// TODO disengage if outside safe limits, unless autonomous flag is set
	// (autonomous flag tells whether user has still possibility to control the airplane,
	// that is he is in the range of radio communication).
class AFCS_FD_Roll: public xf::Module
{
  public:
	// Ctor
	AFCS_FD_Roll (xf::ModuleManager*, QDomElement const& config);

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
	compute_roll();

	/**
	 * Compute roll angle for given PID and measured values and parameters.
	 */
	Optional<Angle>
	compute_roll (xf::PIDControl<double>& pid,
				  xf::Property<Angle> const& cmd_direction,
				  xf::Property<Angle> const& measured_direction,
				  Time const& update_dt) const;

	/**
	 * Called when roll mode property changes.
	 */
	void
	roll_mode_changed();

	/**
	 * Override the "operative" output depending on "autonomous" flag.
	 */
	void
	check_autonomous();

  private:
	xf::PIDControl<double>::Settings	_hdg_pid_settings			= { 1.0, 0.1, 0.0 };
	xf::PIDControl<double>::Settings	_trk_pid_settings			= { 1.0, 0.1, 0.0 };
	xf::PIDControl<double>				_magnetic_hdg_pid			= { _hdg_pid_settings, 0.0 };
	xf::PIDControl<double>				_magnetic_trk_pid			= { _trk_pid_settings, 0.0 };
	xf::RangeSmoother<si::Angle>		_output_roll_smoother		= { { -180.0_deg, +180.0_deg }, 2.5_s };
	afcs_api::RollMode					_roll_mode					= afcs_api::RollMode::None;
	// Input:
	xf::PropertyBoolean					_autonomous;
	xf::PropertyAngle					_roll_limit;
	xf::PropertyInteger					_cmd_roll_mode;
	xf::PropertyAngle					_cmd_magnetic_hdg;
	xf::PropertyAngle					_cmd_magnetic_trk;
	xf::PropertyAngle					_measured_magnetic_hdg;
	xf::PropertyAngle					_measured_magnetic_trk;
	// Output:
	xf::PropertyAngle					_output_roll;
	xf::PropertyBoolean					_operative;
	// Other:
	xf::PropertyObserver				_roll_computer;
};

#endif
