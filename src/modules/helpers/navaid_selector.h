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

#ifndef XEFIS__MODULES__HELPERS__NAVAID_SELECTOR_H__INCLUDED
#define XEFIS__MODULES__HELPERS__NAVAID_SELECTOR_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


class NavaidSelector: public Xefis::Module
{
  public:
	// Ctor
	NavaidSelector (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	/**
	 * Copy left or right property to the output property
	 * depending on the input value (0 or 1). Also check
	 * if property we're going to copy from is fresh.
	 */
	template<class PropertyType>
		static void
		copy (bool selector_fresh, int input, PropertyType& left, PropertyType& right, PropertyType& output);

  private:
	// Input:
	Xefis::PropertyInteger	_selected_input;
	Xefis::PropertyString	_input_l_reference;
	Xefis::PropertyString	_input_l_identifier;
	Xefis::PropertyAngle	_input_l_radial_magnetic;
	Xefis::PropertyAngle	_input_l_reciprocal_magnetic;
	Xefis::PropertyLength	_input_l_distance;
	Xefis::PropertyTime		_input_l_eta;
	Xefis::PropertyString	_input_r_reference;
	Xefis::PropertyString	_input_r_identifier;
	Xefis::PropertyAngle	_input_r_radial_magnetic;
	Xefis::PropertyAngle	_input_r_reciprocal_magnetic;
	Xefis::PropertyLength	_input_r_distance;
	Xefis::PropertyTime		_input_r_eta;
	// Output:
	Xefis::PropertyString	_output_reference;
	Xefis::PropertyString	_output_identifier;
	Xefis::PropertyAngle	_output_radial_magnetic;
	Xefis::PropertyAngle	_output_reciprocal_magnetic;
	Xefis::PropertyLength	_output_distance;
	Xefis::PropertyTime		_output_eta;
};

#endif
