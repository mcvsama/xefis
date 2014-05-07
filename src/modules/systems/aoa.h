/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__SYSTEMS__AOA_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AOA_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>


class AOA: public Xefis::Module
{
  public:
	// Ctor
	AOA (Xefis::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

	void
	compute_critical_aoa();

  private:
	// Settings:
	double						_setting_flaps_factor		= 1.0;
	double						_setting_spoilers_factor	= 1.0;
	Angle						_normal_critical_aoa;
	// Input:
	Xefis::PropertyAngle		_input_flaps_angle;
	Xefis::PropertyAngle		_input_spoilers_angle;
	Xefis::PropertyAngle		_input_aoa_alpha;
	// Output:
	Xefis::PropertyAngle		_output_critical_aoa;
	Xefis::PropertyBoolean		_output_stall;
	// Other:
	Xefis::PropertyObserver		_critical_aoa_computer;
};

#endif
