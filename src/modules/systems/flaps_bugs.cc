/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
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


FlapsBugs::FlapsBugs (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "margin-factor", _margin_factor, false },
	});

	parse_properties (config, {
		{ "input.flaps-setting", _input_flaps_setting, true },
		{ "output.flaps.up.label", _output_flaps_up_label, true },
		{ "output.flaps.up.speed", _output_flaps_up_speed, true },
		{ "output.flaps.a.label", _output_flaps_a_label, true },
		{ "output.flaps.a.speed", _output_flaps_a_speed, true },
		{ "output.flaps.b.label", _output_flaps_b_label, true },
		{ "output.flaps.b.speed", _output_flaps_b_speed, true },
	});
}


void
FlapsBugs::data_updated()
{
	xf::Flaps const& flaps = module_manager()->application()->airframe()->flaps();

	if (_input_flaps_setting.valid())
	{
		if (_input_flaps_setting.fresh())
		{
			_output_flaps_up_label = "UP";
			_output_flaps_up_speed = _margin_factor * flaps.find_setting (0_deg).speed_range().min();

			Optional<std::string> label_a;
			Optional<Speed> speed_a;
			Optional<std::string> label_b;
			Optional<Speed> speed_b;

			auto sett_b = flaps.find_setting (*_input_flaps_setting);
			auto sett_a = sett_b.prev();

			label_b = sett_b.label().toStdString();
			speed_b = _margin_factor * sett_b.speed_range().min();

			if (sett_a)
			{
				label_a = sett_a->label().toStdString();
				speed_a = _margin_factor * sett_a->speed_range().min();
			}

			_output_flaps_a_label = label_a;
			_output_flaps_a_speed = speed_a;
			_output_flaps_b_label = label_b;
			_output_flaps_b_speed = speed_b;
		}
	}
	else
	{
		_output_flaps_a_label.set_nil();
		_output_flaps_a_speed.set_nil();
		_output_flaps_b_label.set_nil();
		_output_flaps_b_speed.set_nil();
	}
}

