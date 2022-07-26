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
#include "flaps_control.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/support/airframe/flaps.h>

// Neutrino:
#include <neutrino/numeric.h>

// Qt:
#include <QTimer>

// Standard:
#include <cstddef>


FlapsControl::FlapsControl (xf::Airframe& airframe, std::string_view const& instance):
	FlapsControlIO (instance)
{
	for (auto s: airframe.flaps().settings())
		_settings_list.insert (s.second.angle());

	if (_settings_list.empty())
		throw xf::BadConfiguration ("missing flaps configuration");

	_timer = std::make_unique<QTimer>();
	_timer->setTimerType (Qt::PreciseTimer);
	_timer->setInterval (kUpdateInterval.in<si::Millisecond>());
	_timer->setSingleShot (false);
	QObject::connect (_timer.get(), &QTimer::timeout, std::bind (&FlapsControl::update_flap_position, this));

	_extents = { *_settings_list.begin(), *_settings_list.rbegin() };
	_current = _extents.min();
}


void
FlapsControl::process (xf::Cycle const&)
{
	if (_input_up_button.value_changed_to (true))
	{
		auto prev_setting = _settings_list.lower_bound (*_io.requested_setting);

		if (prev_setting != _settings_list.begin())
			prev_setting--;

		_io.requested_setting = *prev_setting;
	}
	else if (_input_down_button.value_changed_to (true))
	{
		auto next_setting = _settings_list.upper_bound (*_io.requested_setting);

		if (next_setting != _settings_list.end())
			_io.requested_setting = *next_setting;
	}

	if (_requested_setting.value_changed() && _io.requested_setting)
	{
		_setting = xf::clamped<si::Angle> (*_io.requested_setting, _extents);
		_timer->start();
	}
}


void
FlapsControl::update_flap_position()
{
	using si::abs;

	si::Angle difference = _setting - _current;
	double sgn = xf::sgn (difference.in<si::Degree>());
	si::Angle delta = kUpdateInterval * *_io.angular_velocity;

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

	_io.current = _current;
	_io.control = xf::renormalize (_current, _extents, *_io.control_extents);
}

