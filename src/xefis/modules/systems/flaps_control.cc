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
#include <xefis/core/v2/property_utils.h>
#include <xefis/core/xefis.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/support/airframe/flaps.h>
#include <xefis/utility/numeric.h>

// Local:
#include "flaps_control.h"


FlapsControl::FlapsControl (std::unique_ptr<FlapsControlIO> module_io, xf::Airframe& airframe, std::string const& instance):
	Module (std::move (module_io), instance)
{
	for (auto s: airframe.flaps().settings())
		_settings_list.insert (s.second.angle());

	if (_settings_list.empty())
		throw xf::BadConfiguration ("missing flaps configuration");

	_timer = std::make_unique<QTimer>();
	_timer->setInterval (kUpdateInterval.quantity<Millisecond>());
	_timer->setSingleShot (false);
	QObject::connect (_timer.get(), &QTimer::timeout, std::bind (&FlapsControl::update_flap_position, this));

	_extents = { *_settings_list.begin(), *_settings_list.rbegin() };
	_current = _extents.min();
}


void
FlapsControl::process (v2::Cycle const&)
{
	if (_input_up_clicked())
	{
		auto prev_setting = _settings_list.lower_bound (*io.input_setting);

		if (prev_setting != _settings_list.begin())
			prev_setting--;

		io.input_setting = *prev_setting;
	}
	else if (_input_down_clicked())
	{
		auto next_setting = _settings_list.upper_bound (*io.input_setting);

		if (next_setting != _settings_list.end())
			io.input_setting = *next_setting;
	}

	if (_input_setting_changed() && io.input_setting)
	{
		_setting = xf::clamped<si::Angle> (*io.input_setting, _extents);
		io.output_setting = _setting;
		_timer->start();
	}
}


void
FlapsControl::update_flap_position()
{
	using si::abs;

	double sgn = xf::sgn ((_setting - _current).quantity<Degree>());
	si::Angle difference = _setting - _current;
	si::Angle delta = kUpdateInterval * *io.angular_velocity;

	if (abs (difference) > delta)
	{
		si::Angle diff_to_add = sgn * delta;
		if (abs (diff_to_add) > abs (difference))
			diff_to_add = difference;
		_current += sgn * delta;
	}
	else
	{
		_current = _setting;
		_timer->stop();
	}

	io.current = _current;
	io.control = xf::renormalize (_current, _extents, *io.control_extents);
}

