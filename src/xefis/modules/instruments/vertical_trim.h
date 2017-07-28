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

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/v2/instrument.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>


class VerticalTrimIO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<QString>	label							{ this, "STAB" };

	/*
	 * Input
	 */

	v2::PropertyIn<double>	input_trim_value				{ this, "/trim/value" };
	v2::PropertyIn<double>	input_trim_reference			{ this, "/trim/reference" };
	v2::PropertyIn<double>	input_trim_reference_minimum	{ this, "/trim/reference.minimum" };
	v2::PropertyIn<double>	input_trim_reference_maximum	{ this, "/trim/reference.maximum" };
};


class VerticalTrim:
	public v2::Instrument<VerticalTrimIO>,
	protected xf::InstrumentAids
{
  public:
	// Ctor
	explicit
	VerticalTrim (std::unique_ptr<VerticalTrimIO>, std::string const& instance = {});

	// Module API
	void
	process (v2::Cycle const&) override;

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
	v2::PropertyObserver	_inputs_observer;
};

#endif
