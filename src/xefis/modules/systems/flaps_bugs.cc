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

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "flaps_bugs.h"


FlapsBugs::FlapsBugs (xf::Flaps const& flaps, std::string const& instance):
	Module (instance),
	_flaps (flaps)
{ }


void
FlapsBugs::process (v2::Cycle const&)
{
	if (_flaps_setting_changed())
	{
		if (input_flaps_setting)
		{
			output_flaps_up_label = "UP";
			output_flaps_up_speed = *setting_margin_factor * _flaps.find_setting (0_deg).speed_range().min();

			Optional<std::string> label_a;
			Optional<si::Velocity> speed_a;
			Optional<std::string> label_b;
			Optional<si::Velocity> speed_b;

			auto sett_b = _flaps.find_setting (*input_flaps_setting);
			auto sett_a = sett_b.prev();

			label_b = sett_b.label().toStdString();
			speed_b = *setting_margin_factor * sett_b.speed_range().min();

			if (sett_a)
			{
				label_a = sett_a->label().toStdString();
				speed_a = *setting_margin_factor * sett_a->speed_range().min();
			}

			output_flaps_a_label = label_a;
			output_flaps_a_speed = speed_a;
			output_flaps_b_label = label_b;
			output_flaps_b_speed = speed_b;
		}
		else
		{
			output_flaps_a_label.set_nil();
			output_flaps_a_speed.set_nil();
			output_flaps_b_label.set_nil();
			output_flaps_b_speed.set_nil();
		}
	}
}

