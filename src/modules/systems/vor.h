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

#ifndef XEFIS__MODULES__SYSTEMS__VOR_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__VOR_H__INCLUDED

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


class VOR: public xf::Module
{
  public:
	// Ctor
	VOR (xf::ModuleManager*, QDomElement const& config);

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
	xf::Smoother<double>	_deviation_smoother			= Time (500_ms);
	// Input:
	xf::PropertyAngle		_input_magnetic_declination;
	xf::PropertyAngle		_input_station_latitude;
	xf::PropertyAngle		_input_station_longitude;
	xf::PropertyAngle		_input_aircraft_latitude;
	xf::PropertyAngle		_input_aircraft_longitude;
	xf::PropertyAngle		_input_radial_magnetic;
	// Output:
	xf::PropertyAngle		_output_radial_magnetic;
	xf::PropertyAngle		_output_reciprocal_magnetic;
	xf::PropertyAngle		_output_initial_bearing_magnetic;
	xf::PropertyAngle		_output_deviation;
	xf::PropertyBoolean		_output_to_flag;
	xf::PropertyLength		_output_distance;
	// Other:
	xf::PropertyObserver	_vor_computer;
};

#endif
