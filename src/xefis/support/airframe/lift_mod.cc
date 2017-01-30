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
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/sequence.h>

// Local:
#include "lift_mod.h"


namespace xf {

LiftMod::Setting::Setting (QDomElement const& config)
{
	_label = config.attribute ("label");

	parse (config.attribute ("angle").toStdString(), _angle);

	_speed_range.set_min (parse<Velocity> (config.attribute ("minimum-speed").toStdString()));
	_speed_range.set_max (parse<Velocity> (config.attribute ("maximum-speed").toStdString()));

	parse (config.attribute ("aoa-correction").toStdString(), _aoa_correction);
	_cl_correction = config.attribute ("lift-coefficient-correction").toDouble();
}


void
LiftMod::Setting::link (Setting const* prev, Setting const* next)
{
	_prev = prev;
	_next = next;
}


LiftMod::LiftMod (QDomElement const& config)
{
	// Parse config and populate _settings:
	for (QDomElement const& e: config)
	{
		if (e == "setting")
		{
			Setting setting (e);
			_settings.insert (std::make_pair (setting.angle(), setting));
		}
	}

	if (_settings.empty())
		throw BadConfiguration ("missing configuration");

	// Link all settings to each other, creating a double-linked list:
	for (auto s = _settings.begin(); s != _settings.end(); ++s)
	{
		Setting const* prev = nullptr;
		Setting const* next = nullptr;

		if (s != _settings.begin())
		{
			auto s_prev = s;
			prev = &(--s_prev)->second;
		}

		auto s_next = s;
		if (++s_next != _settings.end())
			next = &s_next->second;

		s->second.link (prev, next);
	}
}


LiftMod::Setting const&
LiftMod::find_setting (Angle const& surface_angle) const
{
	return find_setting_iterator (surface_angle)->second;
}


LiftMod::Setting const*
LiftMod::next_setting (Angle const& surface_angle) const
{
	auto it = find_setting_iterator (surface_angle);
	if (++it != _settings.end())
		return &it->second;
	return nullptr;
}


LiftMod::Setting const*
LiftMod::prev_setting (Angle const& surface_angle) const
{
	auto it = find_setting_iterator (surface_angle);
	if (it != _settings.begin())
		return &(--it)->second;
	return nullptr;
}


Angle
LiftMod::get_aoa_correction (Angle const& surface_angle) const
{
	auto range = extended_adjacent_find (_settings.begin(), _settings.end(), surface_angle, [](Settings::value_type pair) { return pair.first; });

	Range<Angle> from (range.first->first, range.second->first);
	Range<Angle> to (range.first->second.aoa_correction(), range.second->second.aoa_correction());

	return renormalize (surface_angle, from, to);
}


Range<Speed>
LiftMod::get_speed_range (Angle const& surface_angle) const
{
	auto range = extended_adjacent_find (_settings.begin(), _settings.end(), surface_angle, [](Settings::value_type pair) { return pair.first; });

	Range<Angle> from (range.first->first, range.second->first);
	Range<Speed> to_min (range.first->second.speed_range().min(), range.first->second.speed_range().min());
	Range<Speed> to_max (range.first->second.speed_range().max(), range.first->second.speed_range().max());

	return {
		renormalize (surface_angle, from, to_min),
		renormalize (surface_angle, from, to_max),
	};
}


LiftMod::Settings::const_iterator
LiftMod::find_setting_iterator (Angle const& surface_angle) const
{
	using std::abs;

	auto range = extended_adjacent_find (_settings.begin(), _settings.end(), surface_angle, [](Settings::value_type pair) { return pair.first; });

	if (abs (surface_angle - range.first->first) < abs (surface_angle - range.second->first))
		return range.first;
	else
		return range.second;
}

} // namespace xf

