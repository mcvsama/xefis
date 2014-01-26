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

#ifndef XEFIS__MODULES__SYSTEMS__SPEEDS_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__SPEEDS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>


class Speeds: public Xefis::Module
{
  public:
	// Ctor
	Speeds (Xefis::ModuleManager*, QDomElement const& config);

  private:
	// Module API
	void
	data_updated() override;

	void
	compute();

	template<class T>
		static T
		max (Optional<T>, T);

	template<class T>
		static T
		min (Optional<T>, T);

  private:
	Xefis::PropertyObserver	_speeds_computer;
	// Input:
	Xefis::PropertyAngle	_input_flaps_angle;
	// Output:
	Xefis::PropertySpeed	_output_speed_minimum;
	Xefis::PropertySpeed	_output_speed_minimum_maneuver;
	Xefis::PropertySpeed	_output_speed_maximum_maneuver;
	Xefis::PropertySpeed	_output_speed_maximum;
};

#endif
