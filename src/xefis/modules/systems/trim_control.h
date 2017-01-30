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

#ifndef XEFIS__MODULES__SYSTEMS__TRIM_CONTROL_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__TRIM_CONTROL_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>


/**
 * Controls trim value with two buttons or axis.
 * Generates appropriate trimming sound.
 */
class TrimControl: public x2::Module
{
  public:
	/*
	 * Settings
	 */

	x2::Setting<double>		setting_trim_step		{ this, 0.01 };

	/*
	 * Input
	 */

	x2::PropertyIn<double>	input_trim_axis			{ this, "/axis" };
	x2::PropertyIn<double>	input_trim_value		{ this, "/value" };
	x2::PropertyIn<bool>	input_up_trim_button	{ this, "/up-button" };
	x2::PropertyIn<bool>	input_down_trim_button	{ this, "/down-button" };

	/*
	 * Output
	 */

	x2::PropertyOut<double>	output_trim_value		{ this, "/trim-value" };

  public:
	// Ctor
	TrimControl (xf::Xefis*, std::string const& instance = {});

	// Module API
	void
	process (x2::Cycle const&) override;

  private:
	void
	compute_trim();

	void
	update_trim();

	void
	update_trim_without_sound();

	/**
	 * Return true if given button is 'pressed'.
	 */
	static bool
	pressed (x2::Property<bool> const&);

	/**
	 * Return true if given axis is moved 'up'.
	 */
	static bool
	moved_up (x2::Property<double> const&);

	/**
	 * Return true if given axis is moved 'down'.
	 */
	static bool
	moved_down (x2::Property<double> const&);

  private:
	xf::Xefis*				_xefis;
	double					_trim_value		{ 0.0 };
	bool					_trimming_up	{ false };
	bool					_trimming_down	{ false };
	Unique<QTimer>			_timer;
	x2::PropertyObserver	_trim_computer;
	x2::PropertyObserver	_mix_computer;
};

#endif
