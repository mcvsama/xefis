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

#ifndef XEFIS__MODULES__INSTRUMENTS__VERTICAL_TRIM_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__VERTICAL_TRIM_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/instrument.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/instrument/instrument_support.h>
#include <xefis/support/sockets/socket_observer.h>


// TODO handle nans
class VerticalTrimIO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<QString>	label					{ this, "label", "STAB" };

	/*
	 * Input
	 */

	xf::ModuleIn<double>	trim_value				{ this, "trim/value" };
	xf::ModuleIn<double>	trim_reference			{ this, "trim/reference" };
	xf::ModuleIn<double>	trim_reference_minimum	{ this, "trim/reference.minimum" };
	xf::ModuleIn<double>	trim_reference_maximum	{ this, "trim/reference.maximum" };
};


class VerticalTrim:
	public xf::Instrument<VerticalTrimIO>,
	private xf::InstrumentSupport
{
  private:
	struct PaintingParams
	{
		std::optional<QString>	label;
		std::optional<double>	trim_value;
		std::optional<double>	trim_reference;
		std::optional<double>	trim_reference_minimum;
		std::optional<double>	trim_reference_maximum;
	};

  public:
	// Ctor
	explicit
	VerticalTrim (std::unique_ptr<VerticalTrimIO>, xf::Graphics const&, std::string_view const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

	// Instrument API
	std::packaged_task<void()>
	paint (xf::PaintRequest) const override;

  private:
	void
	async_paint (xf::PaintRequest const&, PaintingParams const&) const;

	static QString
	stringify (double value);

  private:
	xf::SocketObserver _inputs_observer;
};

#endif
