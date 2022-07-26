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

#ifndef XEFIS__MODULES__INSTRUMENTS__FLAPS_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__FLAPS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/instrument.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/instrument/instrument_support.h>
#include <xefis/support/sockets/socket_observer.h>

// Standard:
#include <cstddef>


namespace si = neutrino::si;


class FlapsIO: public xf::Instrument
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Angle>	maximum_angle	{ this, "maximum_angle" };
	xf::Setting<bool>		hide_retracted	{ this, "hide_retracted", true };

	/*
	 * Input
	 */

	xf::ModuleIn<si::Angle>	current_angle	{ this, "current-angle" };
	xf::ModuleIn<si::Angle>	set_angle		{ this, "set-angle" };

  public:
	using xf::Instrument::Instrument;
};


// TODO handle nans
class Flaps:
	public FlapsIO,
	private xf::InstrumentSupport
{
  private:
	struct PaintingParams
	{
		si::Angle					maximum_angle;
		bool						hide_retracted;
		std::optional<si::Angle>	current_angle;
		std::optional<si::Angle>	set_angle;
	};

  public:
	// Ctor
	explicit
	Flaps (xf::Graphics const&, std::string_view const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

	// Instrument API
	std::packaged_task<void()>
	paint (xf::PaintRequest) const override;

  private:
	void
	async_paint (xf::PaintRequest const&, PaintingParams const&) const;

  private:
	FlapsIO&			_io { *this };
	xf::SocketObserver	_inputs_observer;
};

#endif
