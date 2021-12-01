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

#ifndef XEFIS__MODULES__INSTRUMENTS__GEAR_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__GEAR_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/instrument.h>
#include <xefis/core/module_io.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/instrument/instrument_support.h>
#include <xefis/support/sockets/socket_observer.h>


// TODO handle nans
class GearIO: public xf::ModuleIO
{
  public:
	/*
	 * Input
	 */

	xf::ModuleIn<bool>	requested_down	{ this, "requested-down" };
	xf::ModuleIn<bool>	nose_up			{ this, "nose-up" };
	xf::ModuleIn<bool>	nose_down		{ this, "nose-down" };
	xf::ModuleIn<bool>	left_up			{ this, "left-up" };
	xf::ModuleIn<bool>	left_down		{ this, "left-down" };
	xf::ModuleIn<bool>	right_up		{ this, "right-up" };
	xf::ModuleIn<bool>	right_down		{ this, "right-down" };
};


class Gear:
	public xf::Instrument<GearIO>,
	private xf::InstrumentSupport
{
  private:
	struct PaintingParams
	{
		std::optional<bool>	requested_down;
		std::optional<bool>	nose_up;
		std::optional<bool>	nose_down;
		std::optional<bool>	left_up;
		std::optional<bool>	left_down;
		std::optional<bool>	right_up;
		std::optional<bool>	right_down;
	};

  public:
	// Ctor
	explicit
	Gear (std::unique_ptr<GearIO>, xf::Graphics const&, std::string_view const& instance = {});

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
	xf::SocketObserver _inputs_observer;
};

#endif
