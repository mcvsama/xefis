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
#include <xefis/core/module.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/sockets/socket_observer.h>
#include <xefis/utility/smoother.h>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class ArrivalETA_IO: public xf::ModuleIO
{
  public:
	/*
	 * Input
	 */

	xf::ModuleIn<si::Angle>	station_latitude	{ this, "station-position/latitude" };
	xf::ModuleIn<si::Angle>	station_longitude	{ this, "station-position/longitude" };
	xf::ModuleIn<si::Angle>	aircraft_latitude	{ this, "aircraft-position/latitude" };
	xf::ModuleIn<si::Angle>	aircraft_longitude	{ this, "aircraft-position/longitude" };
	xf::ModuleIn<si::Angle>	track_lateral_true	{ this, "track-lateral-true" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Time>	eta					{ this, "eta" };
};


class ArrivalETA: public xf::Module<ArrivalETA_IO>
{
  public:
	// Ctor
	explicit
	ArrivalETA (std::unique_ptr<ArrivalETA_IO>, std::string_view const& instance = {});

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

	void
	compute();

  private:
	xf::Smoother<si::Time>		_smoother { 3_s };
	std::optional<si::Length>	_prev_distance;
	xf::SocketObserver			_eta_computer;
};

#endif
