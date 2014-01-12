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

#ifndef XEFIS__MODULES__SYSTEMS__VOR_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__VOR_H__INCLUDED

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


class VOR: public Xefis::Module
{
  public:
	// Ctor
	VOR (Xefis::ModuleManager*, QDomElement const& config);

  private:
	// Module API
	void
	data_updated() override;

	/**
	 * Compute output deviation/flag.
	 */
	void
	compute();

	/**
	 * Normalize to range 0..360_deg.
	 */
	static Angle
	normalize (Angle);

	/**
	 * Denormalize to range -180..180_deg.
	 */
	static Angle
	denormalize (Angle);

  private:
	Xefis::PropertyObserver	_vor_computer;
	Xefis::Smoother<double>	_deviation_smoother			= 500_ms;
	// Input:
	Xefis::PropertyAngle	_input_magnetic_declination;
	Xefis::PropertyAngle	_input_station_latitude;
	Xefis::PropertyAngle	_input_station_longitude;
	Xefis::PropertyAngle	_input_aircraft_latitude;
	Xefis::PropertyAngle	_input_aircraft_longitude;
	Xefis::PropertyAngle	_input_radial_magnetic;
	// Output:
	Xefis::PropertyAngle	_output_radial_magnetic;
	Xefis::PropertyAngle	_output_reciprocal_magnetic;
	Xefis::PropertyAngle	_output_deviation;
	Xefis::PropertyBoolean	_output_to_flag;
};

#endif
