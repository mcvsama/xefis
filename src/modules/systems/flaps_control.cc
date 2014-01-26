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
#include <xefis/airframe/airframe.h>
#include <xefis/airframe/flaps.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/application.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "flaps_control.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/flaps-control", FlapsControl);


constexpr Time FlapsControl::kUpdateInterval;


FlapsControl::FlapsControl (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	QString settings_list_str;

	parse_settings (config, {
		{ "degrees-per-second", _degrees_per_second, false },
		{ "control.minimum", _ctl_minimum, false },
		{ "control.maximum", _ctl_maximum, false },
	});

	parse_properties (config, {
		{ "input.up", _input_up, false },
		{ "input.down", _input_down, false },
		{ "input.setting", _input_setting, true },
		{ "output.setting", _output_setting, false },
		{ "output.current", _output_current, false },
		{ "output.control", _output_control, false },
	});

	for (auto s: module_manager->application()->airframe()->flaps().settings())
		_settings_list.insert (s.second.angle());

	if (_settings_list.empty())
		throw Xefis::Exception ("missing flaps configuration");

	_timer = std::make_unique<QTimer>();
	_timer->setInterval (kUpdateInterval.ms());
	_timer->setSingleShot (false);
	QObject::connect (_timer.get(), &QTimer::timeout, std::bind (&FlapsControl::update_flap_position, this));

	_minimum = *_settings_list.begin();
	_maximum = *_settings_list.rbegin();
	_current = _minimum;
}


void
FlapsControl::data_updated()
{
	if (!_input_setting.valid())
		_input_setting.write (0_deg);

	if (_input_up.fresh() && _input_up.read (false))
	{
		auto prev_setting = _settings_list.lower_bound (_input_setting.read (0_deg));
		if (prev_setting != _settings_list.begin())
			prev_setting--;
		_input_setting.write (*prev_setting);
	}
	else if (_input_down.fresh() && _input_down.read (false))
	{
		auto next_setting = _settings_list.upper_bound (_input_setting.read (0_deg));
		if (next_setting != _settings_list.end())
			_input_setting.write (*next_setting);
	}

	if (_input_setting.fresh() && _input_setting.valid())
	{
		_setting = Xefis::limit (*_input_setting, _minimum, _maximum);
		if (_output_setting.configured())
			_output_setting.write (_setting);
		_timer->start();
	}
}


void
FlapsControl::update_flap_position()
{
	double sgn = Xefis::sgn ((_setting - _current).deg());
	Angle difference = _setting - _current;
	Angle delta = 1_deg * (kUpdateInterval.s() * _degrees_per_second);

	if (std::abs (difference.deg()) > delta.deg())
	{
		Angle diff_to_add = sgn * delta;
		if (std::abs (diff_to_add.deg()) > std::abs (difference.deg()))
			diff_to_add = difference;
		_current += sgn * delta;
	}
	else
	{
		_current = _setting;
		_timer->stop();
	}

	if (_output_current.configured())
		_output_current.write (_current);
	if (_output_control.configured())
		_output_control.write (Xefis::renormalize (_current.deg(), _minimum.deg(), _maximum.deg(), _ctl_minimum, _ctl_maximum));
}

