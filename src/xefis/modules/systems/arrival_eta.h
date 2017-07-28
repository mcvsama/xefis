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

#ifndef XEFIS__MODULES__SYSTEMS__ARRIVAL_ETA_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__ARRIVAL_ETA_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/utility/smoother.h>


class ArrivalETA_IO: public v2::ModuleIO
{
  public:
	/*
	 * Input
	 */

	v2::PropertyIn<si::Angle>	input_station_latitude		{ this, "/station-position/latitude" };
	v2::PropertyIn<si::Angle>	input_station_longitude		{ this, "/station-position/longitude" };
	v2::PropertyIn<si::Angle>	input_aircraft_latitude		{ this, "/aircraft-position/latitude" };
	v2::PropertyIn<si::Angle>	input_aircraft_longitude	{ this, "/aircraft-position/longitude" };
	v2::PropertyIn<si::Angle>	input_track_lateral_true	{ this, "/track-lateral-true" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::Time>	output_eta					{ this, "/eta" };
};


class ArrivalETA: public v2::Module<ArrivalETA_IO>
{
  public:
	// Ctor
	explicit
	ArrivalETA (std::unique_ptr<ArrivalETA_IO>, std::string const& instance = {});

  protected:
	// Module API
	void
	process (v2::Cycle const&) override;

	void
	compute();

  private:
	xf::Smoother<si::Time>		_smoother { 3_s };
	std::optional<si::Length>	_prev_distance;
	v2::PropertyObserver		_eta_computer;
};

#endif
