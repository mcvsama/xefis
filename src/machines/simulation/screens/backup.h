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

#ifndef XEFIS__MACHINES__SIMULATION__SCREENS__BACKUP_H__INCLUDED
#define XEFIS__MACHINES__SIMULATION__SCREENS__BACKUP_H__INCLUDED

// Standard:
#include <cstddef>
#include <memory>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/machine.h>
#include <xefis/core/screen.h>
#include <xefis/modules/instruments/adi.h>


class BackupDisplay: public xf::Screen
{
  public:
	// Ctor
	explicit
	BackupDisplay (xf::ScreenSpec const&, xf::Graphics const& graphics, xf::Machine&, xf::Logger const& logger);

	void
	create_instruments();

  public:
	std::unique_ptr<ADI_IO>				adi_io		{ std::make_unique<ADI_IO>() };

  private:
	xf::Graphics const&					_graphics;
	xf::WorkPerformer					_adi_work_performer;
	// Instruments:
	std::optional<xf::Registrant<ADI>>	_adi;
};

#endif

