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

#ifndef XEFIS__MODULES__SYSTEMS__RCMS_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__RCMS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/sockets/socket_observer.h>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class RemoteControlManagementSystemIO: public xf::ModuleIO
{
  public:
	/*
	 * Input
	 */

	xf::ModuleIn<si::Length>	vlos_caution_distance	{ this, "vlos-caution-distance" };
	xf::ModuleIn<si::Length>	vlos_warning_distance	{ this, "vlos-warning-distance" };
	xf::ModuleIn<si::Angle>		home_longitude			{ this, "home/longitude" };
	xf::ModuleIn<si::Angle>		home_latitude			{ this, "home/latitude" };
	xf::ModuleIn<si::Length>	home_altitude_amsl		{ this, "home/altitude-amsl" };
	xf::ModuleIn<si::Angle>		position_longitude		{ this, "position/longitude" };
	xf::ModuleIn<si::Angle>		position_latitude		{ this, "position/latitude" };
	xf::ModuleIn<si::Length>	position_altitude_amsl	{ this, "position/altitude.amsl" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Length>	distance_vlos			{ this, "distance/vlos" };
	xf::ModuleOut<si::Length>	distance_ground			{ this, "distance/ground" };
	xf::ModuleOut<si::Length>	distance_vertical		{ this, "distance/vertical" };
	xf::ModuleOut<si::Angle>	true_home_direction		{ this, "home-direction.true" };
};


/**
 * Computes VLOS/ground distances and direction to the "home" (base station or whatever
 * configured) from actual basestation and aircraft coordinates.
 */
class RemoteControlManagementSystem: public xf::Module<RemoteControlManagementSystemIO>
{
  public:
	// Ctor
	explicit
	RemoteControlManagementSystem (std::unique_ptr<RemoteControlManagementSystemIO>, std::string_view const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	bool
	home_is_valid() const;

	bool
	position_is_valid() const;

	void
	compute_distances_to_home();

	void
	compute_true_home_direction();

  private:
	xf::SocketObserver _distance_computer;
};

#endif
