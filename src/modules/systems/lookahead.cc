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


Lookahead::Lookahead (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager)
{
	QString input_property_path;
	QString output_property_path;
	double smoothing = 1.f;

	for (QDomElement& e: config)
	{
		if (e == "input")
			input_property_path = e.text();
		else if (e == "output")
			output_property_path = e.text();
		else if (e == "smoothing")
			smoothing = e.text().toDouble();
		else if (e == "minimum-integration-time")
			_minimum_integration_time = 1_s * e.text().toDouble();
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
	_output_smoother.set_samples (smoothing);
}


void
Lookahead::data_updated()
{
	if (_output.is_singular() || !_lookahead_time.valid())
		return;

	if (!_input.valid())
		_output.set_nil();
	else
	{
		Time dt = update_dt();
		_dt += dt;

		if (_dt > _minimum_integration_time)
		{
			double value = *_input;
			double estimated_value = _last_value + *_lookahead_time / dt * (value - _last_value);
			_output.write (_output_smoother.process (estimated_value));
			_last_value = value;
			_dt = Time::epoch();
		}
	}
}

