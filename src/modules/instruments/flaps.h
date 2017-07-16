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

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/instrument.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/v1/property.h>


class Flaps:
	public v1::Instrument,
	protected xf::InstrumentAids
{
  public:
	// Ctor
	Flaps (v1::ModuleManager*, QDomElement const& config);

	void
	data_updated() override;

  protected:
	// QWidget API
	void
	resizeEvent (QResizeEvent*) override;

	// QWidget API
	void
	paintEvent (QPaintEvent*) override;

  private:
	// Settings:
	Angle				_maximum		= 0_deg;
	bool				_hide_retracted	= false;
	// Properties:
	v1::PropertyAngle	_current;
	v1::PropertyAngle	_setting;
};

#endif
