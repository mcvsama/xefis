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
  public:
	// Ctor
	RadialIndicator (Xefis::ModuleManager*, QDomElement const& config);

	void
	data_updated() override;

  private:
	/**
	 * Return Optional<double> from given property
	 * and configured unit name.
	 */
	Optional<double>
	get_optional_value (Xefis::TypedProperty const& property);

  private:
	RadialIndicatorWidget*					_widget = nullptr;
	// Settings:
	bool									_initialize					= true;
	int										_value_precision			= 0;
	unsigned int							_value_modulo				= 0;
	std::string								_unit;
	Xefis::PropertyFloat::Type				_value_minimum;
	Optional<Xefis::PropertyFloat::Type>	_value_maximum_warning;
	Optional<Xefis::PropertyFloat::Type>	_value_maximum_critical;
	Xefis::PropertyFloat::Type				_value_maximum;
	// Properties:
	Xefis::TypedProperty					_value;
	Xefis::TypedProperty					_value_target;
	Xefis::TypedProperty					_value_reference;
};

#endif
