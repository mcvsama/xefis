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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/sockets/socket_observer.h>
#include <xefis/utility/smoother.h>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class VOR_IO: public xf::ModuleIO
{
  public:
	/*
	 * Input
	 */

	xf::ModuleIn<si::Angle>		input_magnetic_declination		{ this, "magnetic-declination" };
	xf::ModuleIn<si::Angle>		input_station_latitude			{ this, "station-position/latitude" };
	xf::ModuleIn<si::Angle>		input_station_longitude			{ this, "station-position/longitude" };
	xf::ModuleIn<si::Angle>		input_aircraft_latitude			{ this, "aircraft-position/latitude" };
	xf::ModuleIn<si::Angle>		input_aircraft_longitude		{ this, "aircraft-position/longitude" };
	xf::ModuleIn<si::Angle>		input_radial_magnetic			{ this, "radial.magnetic" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Angle>	output_radial_magnetic			{ this, "radial.magnetic" };
	xf::ModuleOut<si::Angle>	output_reciprocal_magnetic		{ this, "reciprocal.magnetic" };
	xf::ModuleOut<si::Angle>	output_initial_bearing_magnetic	{ this, "initial-bearing.magnetic" };
	xf::ModuleOut<si::Angle>	output_deviation				{ this, "deviation" };
	xf::ModuleOut<bool>			output_to_flag					{ this, "to-flag" };
	xf::ModuleOut<si::Length>	output_distance					{ this, "distance" };
};


/**
 * Computes information for VOR display (radials, TO/FROM flag, deviation, etc)
 * from actual VOR's coordinates and aircraft coordinates.
 */
class VOR: public xf::Module<VOR_IO>
{
  public:
	// Ctor
	explicit
	VOR (std::unique_ptr<VOR_IO>, std::string_view const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	/**
	 * Compute output deviation/flag.
	 */
	void
	compute();

	/**
	 * Normalize to range 0..360_deg.
	 */
	static si::Angle
	normalize (si::Angle);

	/**
	 * Denormalize to range -180..180_deg.
	 */
	static si::Angle
	denormalize (si::Angle);

  private:
	xf::Smoother<si::Angle>	_deviation_smoother	{ 500_ms };
	xf::SocketObserver		_vor_computer;
};

#endif
