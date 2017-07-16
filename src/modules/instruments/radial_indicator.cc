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
#include "radial_indicator.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/radial-indicator", RadialIndicator)


RadialIndicator::RadialIndicator (v1::ModuleManager* module_manager, QDomElement const& config):
	Instrument (module_manager, config)
{
	_widget = new RadialIndicatorWidget (this);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_widget);

	parse_settings (config, {
		{ "value.precision", _value_precision, false },
		{ "value.modulo", _value_modulo, false },
		{ "value.minimum", _value_minimum, true },
		{ "value.maximum.warning", _value_maximum_warning, false },
		{ "value.maximum.critical", _value_maximum_critical, false },
		{ "value.maximum", _value_maximum, true },
		{ "unit", _unit, false },
	});

	parse_properties (config, {
		{ "value", _value, true },
		{ "value.target", _value_target, false },
		{ "value.reference", _value_reference, false },
		{ "value.automatic", _value_automatic, false },
	});
}


void
RadialIndicator::data_updated()
{
	if (_initialize || _value.fresh() || _value_target.fresh() || _value_reference.fresh() || _value_automatic.fresh())
	{
		_widget->set_range (xf::Range<double> { _value_minimum, _value_maximum });
		_widget->set_precision (_value_precision);
		_widget->set_modulo (_value_modulo);

		_widget->set_value (get_optional_value (_value));
		_widget->set_warning_value (_value_maximum_warning);
		_widget->set_critical_value (_value_maximum_critical);
		_widget->set_target_value (get_optional_value (_value_target));
		_widget->set_reference_value (get_optional_value (_value_reference));
		_widget->set_automatic_value (get_optional_value (_value_automatic));

		_initialize = false;
	}
}


Optional<double>
RadialIndicator::get_optional_value (v1::GenericProperty const& property)
{
	Optional<double> result;
	if (property.valid())
	{
		try {
			result = property.to_float (_unit);
		}
		catch (UnsupportedUnit const&)
		{
			log() << "Unsupported unit '" << _unit << "'." << std::endl;
		}
	}
	return result;
}

