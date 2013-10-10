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

#ifndef XEFIS__MODULES__SYSTEMS__ALTACQ_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__ALTACQ_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/smoother.h>


class AltAcq: public Xefis::Module
{
  public:
	// Ctor
	AltAcq (Xefis::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

	void
	compute_altitude_acquire_distance();

  private:
	Xefis::PropertyObserver		_altitude_acquire_distance_computer;
	Xefis::Smoother<double>		_altitude_acquire_distance_smoother		= 1_s;
	// Input:
	Xefis::PropertyLength		_altitude_amsl;
	Xefis::PropertyLength		_altitude_acquire_amsl;
	Xefis::PropertySpeed		_vertical_speed;
	Xefis::PropertySpeed		_ground_speed;
	// Output:
	Xefis::PropertyLength		_altitude_acquire_distance;
};

#endif
