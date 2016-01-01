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
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


/**
 * Controls trim value with two buttons or axis.
 * Generates appropriate trimming sound.
 */
class TrimControl: public xf::Module
{
  public:
	// Ctor
	TrimControl (xf::ModuleManager*, QDomElement const& config);

  private:
	// Module API
	void
	data_updated() override;

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
	pressed (xf::PropertyBoolean const&);

	/**
	 * Return true if given axis is moved 'up'.
	 */
	static bool
	moved_up (xf::PropertyFloat const&);

	/**
	 * Return true if given axis is moved 'down'.
	 */
	static bool
	moved_down (xf::PropertyFloat const&);

  private:
	double					_trim_step		= 0.01;
	double					_trim_value		= 0.0;
	bool					_trimming_up	= false;
	bool					_trimming_down	= false;
	Unique<QTimer>			_timer;
	// Input:
	xf::PropertyFloat		_input_trim_axis;
	xf::PropertyFloat		_input_trim_value;
	xf::PropertyBoolean		_input_up_trim_button;
	xf::PropertyBoolean		_input_down_trim_button;
	// Output:
	xf::PropertyFloat		_output_trim_value;
	// Other:
	xf::PropertyObserver	_trim_computer;
	xf::PropertyObserver	_mix_computer;
};

#endif
