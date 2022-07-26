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

#ifndef XEFIS__MODULES__SYSTEMS__FLAPS_CONTROL_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__FLAPS_CONTROL_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/support/sockets/socket_value_changed.h>

// Neutrino:
#include <neutrino/range.h>

// Standard:
#include <cstddef>
#include <set>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class FlapsControlIO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	// How fast flaps should extend/retract:
	xf::Setting<si::AngularVelocity>	angular_velocity	{ this, "angular_velocity", 10_deg / 1_s };
	// Range of output_control socket:
	xf::Setting<xf::Range<double>>		control_extents		{ this, "control_extents", { 0.0, 1.0 } };

	/*
	 * Input
	 */

	xf::ModuleIn<bool>					up					{ this, "up", false };
	xf::ModuleIn<bool>					down				{ this, "down", false };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Angle>			requested_setting	{ this, "requested-setting" };
	xf::ModuleOut<si::Angle>			current				{ this, "current" };
	xf::ModuleOut<double>				control				{ this, "control" };

  public:
	using xf::Module::Module;
};


class FlapsControl: public FlapsControlIO
{
	static constexpr si::Time kUpdateInterval = 10_ms;

  public:
	// Ctor
	explicit
	FlapsControl (xf::Airframe&, std::string_view const& instance = {});

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	void
	update_flap_position();

  private:
	FlapsControlIO&						_io					{ *this };
	std::set<si::Angle>					_settings_list;
	xf::Range<si::Angle>				_extents;
	si::Angle							_setting;
	si::Angle							_current;
	std::unique_ptr<QTimer>				_timer;
	xf::SocketValueChanged<bool>		_input_up_button	{ _io.up };
	xf::SocketValueChanged<bool>		_input_down_button	{ _io.down };
	xf::SocketValueChanged<si::Angle>	_requested_setting	{ _io.requested_setting };
};

#endif
