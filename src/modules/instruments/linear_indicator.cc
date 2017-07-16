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

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/services.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "linear_indicator.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/linear-indicator", LinearIndicator)


LinearIndicator::LinearIndicator (v1::ModuleManager* module_manager, QDomElement const& config):
	Instrument (module_manager, config)
{
	_widget = new LinearIndicatorWidget (this);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_widget);

	parse_settings (config, {
		{ "style.mirrored", _style_mirrored, false },
		{ "value.precision", _value_precision, false },
		{ "value.modulo", _value_modulo, false },
		{ "value.digits", _value_digits, false },
		{ "value.minimum", _value_minimum, true },
		{ "value.minimum.critical", _value_minimum_critical, false },
		{ "value.minimum.warning", _value_minimum_warning, false },
		{ "value.maximum.warning", _value_maximum_warning, false },
		{ "value.maximum.critical", _value_maximum_critical, false },
		{ "value.maximum", _value_maximum, true },
		{ "unit", _unit, false },
	});

	parse_properties (config, {
		{ "value", _value, true },
	});
}


void
LinearIndicator::data_updated()
{
	if (_initialize || _value.fresh())
	{
		_widget->set_mirrored_style (_style_mirrored);
		_widget->set_range (xf::Range<double> { _value_minimum, _value_maximum });
		_widget->set_precision (_value_precision);
		_widget->set_modulo (_value_modulo);
		_widget->set_digits (_value_digits);

		Optional<double> value;
		if (_value.valid())
		{
			try {
				value = _value.to_float (_unit);
			}
			catch (UnsupportedUnit const&)
			{
				log() << "Unsupported unit '" << _unit << "'." << std::endl;
			}
		}

		_widget->set_value (value);
		_widget->set_minimum_critical_value (_value_minimum_critical);
		_widget->set_minimum_warning_value (_value_minimum_warning);
		_widget->set_maximum_warning_value (_value_maximum_warning);
		_widget->set_maximum_critical_value (_value_maximum_critical);

		_initialize = false;
	}
}

