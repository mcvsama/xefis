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

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/v1/property.h>


class HorizontalTrim:
	public xf::Instrument,
	protected xf::InstrumentAids
{
  public:
	// Ctor
	HorizontalTrim (xf::ModuleManager*, QDomElement const& config);

	void
	data_updated() override;

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

  private:
	static QString
	stringify (double value);

  private:
	// Settings:
	QString				_label = "TRIM";
	QString				_label_min;
	QString				_label_max;
	// Properties:
	xf::PropertyFloat	_input_trim_value;
	xf::PropertyFloat	_input_trim_reference;
	xf::PropertyFloat	_input_trim_reference_minimum;
	xf::PropertyFloat	_input_trim_reference_maximum;
};

#endif
