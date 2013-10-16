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
	RadialIndicator (Xefis::ModuleManager*, QDomElement const& config);

  public slots:
	/**
	 * Force EFIS to read data from properties.
	 */
	void
	read();

  protected:
	void
	data_updated() override;

  private:
	RadialIndicatorWidget*		_widget = nullptr;
	std::string					_property_path;
	// Settings:
	int							_value_precision			= 0;
	unsigned int				_value_modulo				= 0;
	Xefis::PropertyFloat::Type	_value_minimum;
	Xefis::PropertyFloat::Type	_value_maximum;
	Xefis::PropertyFloat::Type	_value_maximum_warning		= 0.f;
	Xefis::PropertyFloat::Type	_value_maximum_critical		= 0.f;
	// Properties:
	Xefis::PropertyFloat		_value;
	Xefis::PropertyFloat		_value_bug;
	Xefis::PropertyFloat		_value_normal;
};


inline void
RadialIndicator::data_updated()
{
	read();
}

#endif
