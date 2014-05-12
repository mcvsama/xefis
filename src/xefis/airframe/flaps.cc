/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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

// Local:
#include "flaps.h"


namespace Xefis {

Flaps::Setting::Setting (QDomElement const& config)
{
	_label = config.attribute ("label");

	_angle.parse (config.attribute ("angle").toStdString());

	Speed min, max;
	min.parse (config.attribute ("minimum-speed").toStdString());
	max.parse (config.attribute ("maximum-speed").toStdString());
	_speed_range.set_min (min);
	_speed_range.set_max (max);

	_aoa_correction.parse (config.attribute ("aoa-correction").toStdString());
}


void
Flaps::Setting::link (Setting const* prev, Setting const* next)
{
	_prev = prev;
	_next = next;
}


Flaps::Flaps (QDomElement const& config)
{
	for (QDomElement const& e: config)
	{
		if (e == "setting")
		{
			Setting setting (e);
			_settings.insert (std::make_pair (setting.angle(), setting));
		}
	}

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


Flaps::Setting const&
Flaps::find_setting (Angle const& flaps_angle) const
{
	return find_setting_iterator (flaps_angle)->second;
}


Flaps::Setting const*
Flaps::next_setting (Angle const& flaps_angle) const
{
	auto it = find_setting_iterator (flaps_angle);
	if (++it != _settings.end())
		return &it->second;
	return nullptr;
}


Flaps::Setting const*
Flaps::prev_setting (Angle const& flaps_angle) const
{
	auto it = find_setting_iterator (flaps_angle);
	if (it != _settings.begin())
		return &(--it)->second;
	return nullptr;
}


Angle
Flaps::get_aoa_correction (Angle const& flaps_angle) const
{
	auto ub = _settings.upper_bound (flaps_angle);
	auto lb = ub;
	if (lb != _settings.begin())
		--lb;

	if (lb == _settings.end())
		return ub->second.aoa_correction();
	else if (ub == _settings.end())
		return lb->second.aoa_correction();
	else
	{
		Range<double> from (lb->first.deg(), ub->first.deg());
		Range<double> to (lb->second.aoa_correction().deg(), ub->second.aoa_correction().deg());

		return 1_deg * renormalize (flaps_angle.deg(), from, to);
	}
}


Range<Speed>
Flaps::get_speed_range (Angle const& flaps_angle) const
{
	if (_settings.empty())
		throw BadConfiguration ("missing flaps configuration");

	auto ub = _settings.upper_bound (flaps_angle);
	auto lb = ub;

	if (lb != _settings.begin())
		--lb;

	if (ub == _settings.end())
		--ub;

	Range<double> from (lb->first.deg(), ub->first.deg());
	Range<double> to_min (lb->second.speed_range().min().kt(), ub->second.speed_range().min().kt());
	Range<double> to_max (lb->second.speed_range().max().kt(), ub->second.speed_range().max().kt());

	return {
		1_kt * renormalize (flaps_angle.deg(), from, to_min),
		1_kt * renormalize (flaps_angle.deg(), from, to_max),
	};
}


Flaps::Settings::const_iterator
Flaps::find_setting_iterator (Angle const& flaps_angle) const
{
	if (_settings.empty())
		throw BadConfiguration ("missing flaps configuration");

	// Find Setting for given flaps_angle.
	auto ub = _settings.upper_bound (flaps_angle);
	auto lb = ub;
	if (lb != _settings.begin())
		--lb;

	if (lb == _settings.end())
		return ub;
	else if (ub == _settings.end())
		return lb;
	else
	{
		if (std::abs (flaps_angle - lb->first) < std::abs (flaps_angle - ub->first))
			return lb;
		else
			return ub;
	}
}

} // namespace Xefis

