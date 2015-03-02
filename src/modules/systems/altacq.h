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

#ifndef XEFIS__MODULES__SYSTEMS__ALTACQ_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__ALTACQ_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/smoother.h>


class AltAcq: public xf::Module
{
  public:
	// Ctor
	AltAcq (xf::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

	void
	compute_altitude_acquire_distance();

  private:
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	xf::Smoother<double>	_altitude_acquire_distance_smoother	= 2_s;
	Length					_minimum_altitude_difference;
	Length					_flag_diff_on						= 1000_ft;
	Length					_flag_diff_off						= 100_ft;
	bool					_flag_armed							= false;
	// Input:
	xf::PropertyLength		_altitude_amsl;
	xf::PropertyLength		_altitude_acquire_amsl;
	Time					_altitude_acquire_amsl_timestamp;
	xf::PropertySpeed		_vertical_speed;
	xf::PropertySpeed		_ground_speed;
	// Output:
	xf::PropertyLength		_altitude_acquire_distance;
	xf::PropertyBoolean		_altitude_acquire_flag;
	// Other:
	xf::PropertyObserver	_altitude_acquire_distance_computer;
};

#endif
