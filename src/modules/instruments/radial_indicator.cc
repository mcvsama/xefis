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


XEFIS_REGISTER_MODULE_CLASS ("instruments/radial-indicator", RadialIndicator);


RadialIndicator::RadialIndicator (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Xefis::Instrument (module_manager, config)
{
	_widget = new RadialIndicatorWidget (this);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_widget);

	bool found_properties_config = false;

	for (QDomElement& e: config)
	{
		if (e == "settings")
		{
			parse_settings (e, {
				{ "value.precision", _value_precision, false },
				{ "value.modulo", _value_modulo, false },
				{ "value.minimum", _value_minimum, true },
				{ "value.maximum", _value_maximum, true },
				{ "value.maximum.warning", _value_maximum_warning, false },
				{ "value.maximum.critical", _value_maximum_critical, false },
			});
		}
		else if (e == "properties")
		{
			parse_properties (e, {
				{ "value", _value, true },
				{ "value.bug", _value_bug, false },
				{ "value.normal", _value_normal, false },
			});
			found_properties_config = true;
		}
	}

	if (!found_properties_config)
		throw Xefis::Exception ("module configuration missing");
}


void
RadialIndicator::read()
{
	_widget->set_range (Xefis::Range<double> { _value_minimum, _value_maximum });
	_widget->set_precision (_value_precision);
	_widget->set_modulo (_value_modulo);

	_widget->set_value (*_value);
	_widget->set_value_visible (_value.valid());

	_widget->set_target_value (*_value_bug);
	_widget->set_target_visible (_value_bug.valid());

	_widget->set_warning_value (_value_maximum_warning);
	_widget->set_warning_visible (has_setting ("value.maximum.warning"));

	_widget->set_critical_value (_value_maximum_critical);
	_widget->set_critical_visible (has_setting ("value.maximum.critical"));

	_widget->set_normal_value (*_value_normal);
	_widget->set_normal_visible (_value_normal.valid());
}

