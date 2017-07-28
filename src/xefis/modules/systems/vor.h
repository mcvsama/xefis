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
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/utility/smoother.h>


class VOR_IO: public v2::ModuleIO
{
  public:
	/*
	 * Input
	 */

	v2::PropertyIn<si::Angle>	input_magnetic_declination		{ this, "/magnetic-declination" };
	v2::PropertyIn<si::Angle>	input_station_latitude			{ this, "/station-position/latitude" };
	v2::PropertyIn<si::Angle>	input_station_longitude			{ this, "/station-position/longitude" };
	v2::PropertyIn<si::Angle>	input_aircraft_latitude			{ this, "/aircraft-position/latitude" };
	v2::PropertyIn<si::Angle>	input_aircraft_longitude		{ this, "/aircraft-position/longitude" };
	v2::PropertyIn<si::Angle>	input_radial_magnetic			{ this, "/radial.magnetic" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::Angle>	output_radial_magnetic			{ this, "/radial.magnetic" };
	v2::PropertyOut<si::Angle>	output_reciprocal_magnetic		{ this, "/reciprocal.magnetic" };
	v2::PropertyOut<si::Angle>	output_initial_bearing_magnetic	{ this, "/initial-bearing.magnetic" };
	v2::PropertyOut<si::Angle>	output_deviation				{ this, "/deviation" };
	v2::PropertyOut<bool>		output_to_flag					{ this, "/to-flag" };
	v2::PropertyOut<si::Length>	output_distance					{ this, "/distance" };
};


class VOR: public v2::Module<VOR_IO>
{
  public:
	// Ctor
	explicit
	VOR (std::unique_ptr<VOR_IO>, std::string const& instance = {});

	// Module API
	void
	process (v2::Cycle const&) override;

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
	v2::PropertyObserver	_vor_computer;
};

#endif
