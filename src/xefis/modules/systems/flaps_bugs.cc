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

// Local:
#include "flaps_bugs.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


FlapsBugs::FlapsBugs (xf::Flaps const& flaps, std::string_view const& instance):
	FlapsBugsIO (instance),
	_flaps (flaps)
{ }


void
FlapsBugs::process (xf::Cycle const&)
{
	if (_flaps_setting_changed.value_changed())
	{
		if (_io.flaps_setting)
		{
			_io.flaps_up_label = "UP";
			_io.flaps_up_speed = *_io.margin_factor * _flaps.find_setting (0_deg).speed_range().min();

			std::optional<std::string> label_a;
			std::optional<si::Velocity> speed_a;
			std::optional<std::string> label_b;
			std::optional<si::Velocity> speed_b;

			auto sett_b = _flaps.find_setting (*_io.flaps_setting);
			auto sett_a = sett_b.prev();

			label_b = sett_b.label().toStdString();
			speed_b = *_io.margin_factor * sett_b.speed_range().min();

			if (sett_a)
			{
				label_a = sett_a->label().toStdString();
				speed_a = *_io.margin_factor * sett_a->speed_range().min();
			}

			_io.flaps_a_label = label_a;
			_io.flaps_a_speed = speed_a;
			_io.flaps_b_label = label_b;
			_io.flaps_b_speed = speed_b;
		}
		else
		{
			_io.flaps_a_label = xf::nil;
			_io.flaps_a_speed = xf::nil;
			_io.flaps_b_label = xf::nil;
			_io.flaps_b_speed = xf::nil;
		}
	}
}

