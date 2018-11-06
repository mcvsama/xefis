/* vim:ts=4
 *
 * Copyleft 2008…2017  Michał Gawron
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
#include "backup.h"


BackupDisplay::BackupDisplay (xf::ScreenSpec const& spec, xf::Graphics const& graphics, xf::Machine& machine, xf::Logger const& logger):
	Screen (spec, graphics, machine, "Backup Display", logger),
	_graphics (graphics),
	_adi_work_performer (1, logger.with_scope ("ADI"))
{
	adi_io->speed_ladder_line_every			= 10;
	adi_io->speed_ladder_number_every		= 20;
	adi_io->speed_ladder_extent				= 124;
	adi_io->speed_ladder_minimum			= 0;
	adi_io->speed_ladder_maximum			= 999;
	adi_io->altitude_ladder_line_every		= 100;
	adi_io->altitude_ladder_number_every	= 200;
	adi_io->altitude_ladder_emphasis_every	= 1000;
	adi_io->altitude_ladder_bold_every		= 500;
	adi_io->altitude_ladder_extent			= 825;
	adi_io->altitude_landing_warning_hi		= 1000_ft;
	adi_io->altitude_landing_warning_lo		= 500_ft;
	adi_io->enable_raising_runway			= true;
	adi_io->raising_runway_visibility		= 1000_ft;
	adi_io->raising_runway_threshold		= 250_ft;
	adi_io->aoa_visibility_threshold		= 17.5_deg;
	adi_io->show_mach_above					= 0.4;
	adi_io->power_eq_1000_fpm				= 1000_W;
	adi_io->show_vertical_speed_ladder		= false;
}


void
BackupDisplay::create_instruments()
{
	_adi.emplace (std::move (adi_io), _graphics, "backup-adi");
	register_instrument (*_adi, _adi_work_performer);
	set (**_adi, { 0.0, 0.1, 1.0, 0.8 });
}


