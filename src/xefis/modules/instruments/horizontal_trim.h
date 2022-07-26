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

#ifndef XEFIS__MODULES__INSTRUMENTS__HORIZONTAL_TRIM_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__HORIZONTAL_TRIM_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/instrument.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/instrument/instrument_support.h>
#include <xefis/support/sockets/socket_observer.h>

// Standard:
#include <cstddef>


// TODO handle nans
class HorizontalTrimIO: public xf::Instrument
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<QString>	label					{ this, "label", "TRIM" };
	xf::Setting<QString>	label_min				{ this, "label_min", "" };
	xf::Setting<QString>	label_max				{ this, "label_max", "" };

	/*
	 * Input
	 */

	xf::ModuleIn<double>	trim_value				{ this, "trim/value" };
	xf::ModuleIn<double>	trim_reference			{ this, "trim/reference" };
	xf::ModuleIn<double>	trim_reference_minimum	{ this, "trim/reference.minimum" };
	xf::ModuleIn<double>	trim_reference_maximum	{ this, "trim/reference.maximum" };

  public:
	using xf::Instrument::Instrument;
};


class HorizontalTrim:
	public HorizontalTrimIO,
	private xf::InstrumentSupport
{
  private:
	struct PaintingParams
	{
		std::optional<QString>	label;
		std::optional<QString>	label_min;
		std::optional<QString>	label_max;
		std::optional<double>	trim_value;
		std::optional<double>	trim_reference;
		std::optional<double>	trim_reference_minimum;
		std::optional<double>	trim_reference_maximum;
	};

  public:
	// Ctor
	explicit
	HorizontalTrim (xf::Graphics const&, std::string_view const& instance = {});

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
	HorizontalTrimIO&	_io { *this };
	xf::SocketObserver	_inputs_observer;
};

#endif
