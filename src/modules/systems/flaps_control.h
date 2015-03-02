/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__SYSTEMS__FLAPS_CONTROL_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__FLAPS_CONTROL_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Qt:
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


/**
 * Controls extending/retracting flaps with configured speed.
 * Reacts to buttons to change to next or previous setting.
 */
class FlapsControl: public xf::Module
{
	static constexpr Time kUpdateInterval = 10_ms;

  public:
	// Ctor
	FlapsControl (xf::ModuleManager*, QDomElement const& config);

  protected:
	// Module API
	void
	data_updated() override;

  private:
	void
	update_flap_position();

  private:
	std::set<Angle>			_settings_list;
	double					_ctl_minimum		= 0.0;
	double					_ctl_maximum		= 1.0;
	Angle					_minimum;
	Angle					_maximum;
	Angle					_setting;
	Angle					_current;
	double					_degrees_per_second	= 100.0;
	// Input:
	xf::PropertyBoolean		_input_up;
	xf::PropertyBoolean		_input_down;
	xf::PropertyAngle		_input_setting;
	// Output:
	xf::PropertyAngle		_output_setting;
	xf::PropertyAngle		_output_current;
	xf::PropertyFloat		_output_control;
	// Other:
	Unique<QTimer>			_timer;
};

#endif
