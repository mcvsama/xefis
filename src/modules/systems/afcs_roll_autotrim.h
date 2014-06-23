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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_ROLL_AUTOTRIM_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_ROLL_AUTOTRIM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


/**
 * Compute ailerons correction to apply to counter react engine's torque.
 * Depends on airspeed and engine RPM. Factors need to be obtained experimentally.
 *
 * Works only for air speeds well below Mach 1.
 */
class AFCS_RollAutotrim: public Xefis::Module
{
  public:
	// Ctor
	AFCS_RollAutotrim (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	// Settings:
	double				_airspeed_coefficient		= 0.0;
	double				_engine_torque_coefficient	= 0.0;
	double				_total_coefficient			= 1.0;
	// Input:
	xf::PropertySpeed	_input_airspeed;
	xf::PropertyTorque	_input_engine_torque;
	// Output:
	xf::PropertyFloat	_output_ailerons_correction;
};

#endif
