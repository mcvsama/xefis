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

#ifndef XEFIS__MODULES__SYSTEMS__FLAPS_BUGS_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__FLAPS_BUGS_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Qt:
#include <QtCore/QTimer>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


/**
 * Computes two speed bugs - for two adjacent flap settings - that should
 * be displayed on EFIS' speed ladder.
 */
class FlapsBugs: public Xefis::Module
{
  public:
	// Ctor
	FlapsBugs (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	// Module API
	void
	data_updated() override;

  private:
	// Settings:
	double					_margin_factor = 1.2;
	// Input:
	Xefis::PropertyAngle	_input_flaps_setting;
	// Output:
	Xefis::PropertyString	_output_flaps_up_label;
	Xefis::PropertySpeed	_output_flaps_up_speed;
	Xefis::PropertyString	_output_flaps_a_label;
	Xefis::PropertySpeed	_output_flaps_a_speed;
	Xefis::PropertyString	_output_flaps_b_label;
	Xefis::PropertySpeed	_output_flaps_b_speed;
};

#endif
