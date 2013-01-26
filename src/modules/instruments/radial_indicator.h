/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__INSTRUMENTS__RADIAL_INDICATOR_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__RADIAL_INDICATOR_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/core/property.h>

// Local:
#include "radial_indicator_widget.h"


class RadialIndicator: public Xefis::Instrument
{
	Q_OBJECT

  public:
	// Ctor
	RadialIndicator (Xefis::ModuleManager*, QDomElement const& config, QWidget* parent);

  public slots:
	/**
	 * Force EFIS to read data from properties.
	 */
	void
	read();

  protected:
	void
	data_update() override;

  private:
	RadialIndicatorWidget*	_widget = nullptr;
	std::string				_property_path;

	Xefis::PropertyFloat	_value;
	Xefis::PropertyFloat	_range_minimum;
	Xefis::PropertyFloat	_range_maximum;
	Xefis::PropertyFloat	_warning_value;
	Xefis::PropertyFloat	_critical_value;
	Xefis::PropertyFloat	_normal_value;
};


inline void
RadialIndicator::data_update()
{
	read();
}

#endif
