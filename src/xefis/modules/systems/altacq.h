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

#ifndef XEFIS__MODULES__SYSTEMS__ALTACQ_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__ALTACQ_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/sockets/socket_observer.h>
#include <xefis/support/sockets/socket_value_changed.h>
#include <xefis/utility/smoother.h>

// Standard:
#include <cstddef>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class AltAcqIO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Length>		minimum_altitude_difference	{ this, "minimum_altitude_difference" };
	xf::Setting<si::Length>		flag_diff_on				{ this, "flag_diff_on", 1000_ft };
	xf::Setting<si::Length>		flag_diff_off				{ this, "flag_diff_off", 100_ft };

	/*
	 * Input
	 */

	xf::ModuleIn<si::Length>	altitude_amsl				{ this, "altitude-amsl" };
	xf::ModuleIn<si::Length>	altitude_acquire_amsl		{ this, "altitude-acquire-amsl" };
	xf::ModuleIn<si::Velocity>	vertical_speed				{ this, "vertical-speed" };
	xf::ModuleIn<si::Velocity>	ground_speed				{ this, "ground-speed" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Length>	altitude_acquire_distance	{ this, "acquire-distance" };
	xf::ModuleOut<bool>			altitude_acquire_flag		{ this, "acquire-flag" };

  public:
	using xf::Module::Module;
};


class AltAcq: public AltAcqIO
{
  public:
	// Ctor
	explicit
	AltAcq (std::string_view const& instance = {});

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

	void
	compute_altitude_acquire_distance();

  private:
	AltAcqIO&							_io									{ *this };
	bool								_flag_armed							{ false };
	// Note: SocketObservers depend on Smoothers, so first Smoothers must be defined,
	// then SocketObservers, to ensure correct order of destruction.
	xf::Smoother<si::Length>			_output_smoother					{ 2_s };
	xf::SocketObserver					_output_computer;
	xf::SocketValueChanged<si::Length>	_altitude_amsl_changed				{ _io.altitude_amsl };
	xf::SocketValueChanged<si::Length>	_altitude_acquire_amsl_changed		{ _io.altitude_acquire_amsl };
};

#endif
