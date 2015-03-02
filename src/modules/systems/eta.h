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

#ifndef XEFIS__MODULES__SYSTEMS__ETA_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__ETA_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/smoother.h>


class ETA: public xf::Module
{
  public:
	// Ctor
	ETA (xf::ModuleManager*, QDomElement const& config);

  private:
	// Module API
	void
	data_updated() override;

	void
	compute();

  private:
	xf::Smoother<double>	_smoother = 3_s;
	Optional<Length>		_prev_distance;
	// Input:
	xf::PropertyAngle		_input_station_latitude;
	xf::PropertyAngle		_input_station_longitude;
	xf::PropertyAngle		_input_aircraft_latitude;
	xf::PropertyAngle		_input_aircraft_longitude;
	xf::PropertyAngle		_input_track_lateral_true;
	// Output:
	xf::PropertyTime		_output_eta;
	// Other:
	xf::PropertyObserver	_eta_computer;
};

#endif
