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
#include <xefis/core/instrument.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/core/setting.h>
#include <xefis/support/instrument/instrument_support.h>


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

	xf::PropertyIn<double>	trim_value				{ this, "trim/value" };
	xf::PropertyIn<double>	trim_reference			{ this, "trim/reference" };
	xf::PropertyIn<double>	trim_reference_minimum	{ this, "trim/reference.minimum" };
	xf::PropertyIn<double>	trim_reference_maximum	{ this, "trim/reference.maximum" };
};


class VerticalTrim:
	public xf::Instrument<VerticalTrimIO>,
	private xf::InstrumentSupport
{
  public:
	// Ctor
	explicit
	VerticalTrim (std::unique_ptr<VerticalTrimIO>, std::string const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

	// Instrument API
	void
	paint (xf::PaintRequest&) const override;

  private:
	static QString
	stringify (double value);

  private:
	xf::PropertyObserver	_inputs_observer;
};

#endif
