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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/application.h>
#include <xefis/core/module_manager.h>
#include <xefis/utility/qdom.h>
#include <xefis/airframe/airframe.h>
#include <xefis/airframe/flaps.h>

// Local:
#include "flaps_bugs.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/flaps-bugs", FlapsBugs);


FlapsBugs::FlapsBugs (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_properties (config, {
		{ "input.flaps-setting", _input_flaps_setting, true },
		{ "output.flaps-bug.label", _output_bug_label, true },
		{ "output.flaps-bug.speed", _output_bug_speed, true },
	});
}


void
FlapsBugs::data_updated()
{
	Xefis::Flaps const& flaps = module_manager()->application()->airframe()->flaps();

	if (_input_flaps_setting.valid())
	{
		if (_input_flaps_setting.fresh())
		{
			Xefis::Flaps::Settings::const_iterator s1 = flaps.find_setting_iterator (*_input_flaps_setting);
			Xefis::Flaps::Settings::const_iterator s2 = s1;
			std::advance (s2, std::min<int> (1, std::distance (s2, flaps.settings().end())));
			Xefis::Flaps::Settings::const_iterator end = flaps.settings().end();

			Optional<std::string> label;
			Optional<Speed> speed;

			if (s1 != end)
				label = s1->second.label().toStdString();
			else if (s2 != end)
				label = s2->second.label().toStdString();

			if (s2 != end)
				speed = s2->second.speed_range().max();
			else if (s1 != end)
				speed = s1->second.speed_range().min();

			_output_bug_label = label;
			_output_bug_speed = speed;
		}
	}
	else
	{
		_output_bug_label.set_nil();
		_output_bug_speed.set_nil();
	}
}

