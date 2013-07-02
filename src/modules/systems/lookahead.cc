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
#include <QtCore/QTime>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>

// Local:
#include "lookahead.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/lookahead", Lookahead);


Lookahead::Lookahead (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager),
	_output_estimator (1_s)
{
	QString input_property_path;
	QString output_property_path;
	Time smoothing = 1_ms;

	for (QDomElement& e: config)
	{
		if (e == "input")
			input_property_path = e.text();
		else if (e == "output")
			output_property_path = e.text();
		else if (e == "smoothing")
			smoothing.parse (e.text().toStdString());
		else if (e == "minimum-integration-time")
		{
			Time t;
			t.parse (e.text().toStdString());
			_output_estimator.set_minimum_integration_time (t);
		}
		else if (e == "properties")
		{
			parse_properties (e, {
				{ "lookahead-time", _lookahead_time, true }
			});
		}
	}

	if (input_property_path.isEmpty())
		throw Xefis::Exception ("missing input property config");
	if (output_property_path.isEmpty())
		throw Xefis::Exception ("missing output property config");

	_input = Xefis::PropertyFloat (input_property_path.toStdString());
	_output = Xefis::PropertyFloat (output_property_path.toStdString());
	_output_smoother.set_smoothing_time (smoothing);
}


void
Lookahead::data_updated()
{
	if (_output.is_singular() || !_lookahead_time.valid())
	{
		_output_smoother.invalidate();
		_output_estimator.invalidate();
		return;
	}

	if (!_input.valid())
	{
		_output_smoother.invalidate();
		_output_estimator.invalidate();
		_output.set_nil();
	}
	else
		_output.write (_output_estimator.process (_output_smoother.process (*_input, update_dt()), update_dt()));
}

