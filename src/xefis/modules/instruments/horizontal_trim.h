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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/core/setting.h>


class HorizontalTrimIO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<QString>	label					{ this, "label", "TRIM" };
	xf::Setting<QString>	label_min				{ this, "label_min" };
	xf::Setting<QString>	label_max				{ this, "label_max" };

	/*
	 * Input
	 */

	xf::PropertyIn<double>	trim_value				{ this, "/trim/value" };
	xf::PropertyIn<double>	trim_reference			{ this, "/trim/reference" };
	xf::PropertyIn<double>	trim_reference_minimum	{ this, "/trim/reference.minimum" };
	xf::PropertyIn<double>	trim_reference_maximum	{ this, "/trim/reference.maximum" };
};


class HorizontalTrim:
	public xf::Instrument<HorizontalTrimIO>,
	protected xf::InstrumentAids
{
  public:
	// Ctor
	explicit
	HorizontalTrim (std::unique_ptr<HorizontalTrimIO>, std::string const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

  protected:
	// QWidget API
	void
	resizeEvent (QResizeEvent*) override;

	// QWidget API
	void
	paintEvent (QPaintEvent*) override;

  private:
	static QString
	stringify (double value);

  private:
	xf::PropertyObserver	_inputs_observer;
};

#endif
