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
#include <xefis/support/ui/sound_manager.h>

// Local:
#include "trim_control.h"


TrimControl::TrimControl (std::unique_ptr<TrimControlIO> module_io, xf::Xefis* xefis, std::string const& instance):
	Module (std::move (module_io), instance),
	_xefis (xefis)
{
	_timer = std::make_unique<QTimer>();
	_timer->setInterval (180);
	_timer->setSingleShot (false);
	QObject::connect (_timer.get(), &QTimer::timeout, std::bind (&TrimControl::update_trim, this));

	_trim_computer.set_callback (std::bind (&TrimControl::compute_trim, this));
	_trim_computer.observe ({
		&io.input_trim_axis,
		&io.input_trim_value,
		&io.input_up_trim_button,
		&io.input_down_trim_button,
	});

	update_trim_without_sound();
}


void
TrimControl::process (v2::Cycle const& cycle)
{
	_trim_computer.process (cycle.update_time());
}


void
TrimControl::compute_trim()
{
	if (io.input_trim_value)
		io.output_trim_value = *io.input_trim_value;
	else
	{
		_trimming_up = false;
		_trimming_down = false;

		if (pressed (io.input_up_trim_button))
			_trimming_up = true;

		if (pressed (io.input_down_trim_button))
			_trimming_down = true;

		if (io.input_trim_axis)
		{
			if (moved_up (io.input_trim_axis))
				_trimming_up = true;
			else if (moved_down (io.input_trim_axis))
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
	_xefis->sound_manager()->play (XEFIS_SHARED_DIRECTORY "/sounds/trim-bip.wav");
}


void
TrimControl::update_trim_without_sound()
{
	_trim_value = xf::clamped (_trim_value + (_trimming_up ? 1 : _trimming_down ? -1 : 0) * *io.setting_trim_step, -1.0, 1.0);
	io.output_trim_value = _trim_value;
}


inline bool
TrimControl::pressed (v2::Property<bool> const& property)
{
	return property && *property;
}


inline bool
TrimControl::moved_up (v2::Property<double> const& property)
{
	return property.valid() && *property > 0.5;
}


inline bool
TrimControl::moved_down (v2::Property<double> const& property)
{
	return property.valid() && *property < -0.5;
}

