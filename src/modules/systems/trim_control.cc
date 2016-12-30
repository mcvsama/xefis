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

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/sound_manager.h>

// Local:
#include "trim_control.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/trim-control", TrimControl)


TrimControl::TrimControl (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "trim-step", _trim_step, false },
	});

	parse_properties (config, {
		{ "input.trim-axis", _input_trim_axis, false },
		{ "input.trim-value", _input_trim_value, false },
		{ "input.up-trim-button", _input_up_trim_button, false },
		{ "input.down-trim-button", _input_down_trim_button, false },
		{ "output.trim-value", _output_trim_value, false },
	});

	_timer = std::make_unique<QTimer>();
	_timer->setInterval (180);
	_timer->setSingleShot (false);
	QObject::connect (_timer.get(), &QTimer::timeout, std::bind (&TrimControl::update_trim, this));

	_trim_computer.set_callback (std::bind (&TrimControl::compute_trim, this));
	_trim_computer.observe ({
		&_input_trim_axis,
		&_input_trim_value,
		&_input_up_trim_button,
		&_input_down_trim_button,
	});

	update_trim_without_sound();
}


void
TrimControl::data_updated()
{
	_trim_computer.data_updated (update_time());
}


void
TrimControl::compute_trim()
{
	if (_input_trim_value.fresh())
	{
		_output_trim_value = *_input_trim_value;
	}
	else
	{
		_trimming_up = false;
		_trimming_down = false;

		if (_input_up_trim_button.fresh() && pressed (_input_up_trim_button))
			_trimming_up = true;

		if (_input_down_trim_button.fresh() && pressed (_input_down_trim_button))
			_trimming_down = true;

		if (_input_trim_axis.fresh())
		{
			if (moved_up (_input_trim_axis))
				_trimming_up = true;
			else if (moved_down (_input_trim_axis))
				_trimming_down = true;
		}

		if (_trimming_up || _trimming_down)
		{
			_timer->start();
			update_trim();
		}
		else
			_timer->stop();
	}
}


void
TrimControl::update_trim()
{
	update_trim_without_sound();
	module_manager()->xefis()->sound_manager()->play (XEFIS_SHARED_DIRECTORY "/sounds/trim-bip.wav");
}


void
TrimControl::update_trim_without_sound()
{
	_trim_value = xf::clamped (_trim_value + (_trimming_up ? 1 : _trimming_down ? -1 : 0) * _trim_step, -1.0, 1.0);
	_output_trim_value = _trim_value;
}


inline bool
TrimControl::pressed (xf::PropertyBoolean const& property)
{
	return property.valid() && *property;
}


inline bool
TrimControl::moved_up (xf::PropertyFloat const& property)
{
	return property.valid() && *property > 0.5;
}


inline bool
TrimControl::moved_down (xf::PropertyFloat const& property)
{
	return property.valid() && *property < -0.5;
}

