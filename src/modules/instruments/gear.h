/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/property.h>


class Gear:
	public Xefis::Instrument,
	public Xefis::InstrumentAids
{
  public:
	// Ctor
	Gear (Xefis::ModuleManager*, QDomElement const& config);

	void
	data_updated() override;

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

  private:
	// Properties:
	Xefis::PropertyBoolean	_setting_down;
	Xefis::PropertyBoolean	_nose_up;
	Xefis::PropertyBoolean	_nose_down;
	Xefis::PropertyBoolean	_left_up;
	Xefis::PropertyBoolean	_left_down;
	Xefis::PropertyBoolean	_right_up;
	Xefis::PropertyBoolean	_right_down;
};

#endif
