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

#ifndef XEFIS__MODULES__INSTRUMENTS__FLAPS_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__FLAPS_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/instrument.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>


class Flaps:
	public v2::Instrument,
	protected xf::InstrumentAids
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<si::Angle>		maximum_angle	{ this };
	v2::Setting<bool>			hide_retracted	{ this, true };

	/*
	 * Input
	 */

	v2::PropertyIn<si::Angle>	current_angle	{ this, "/current-angle" };
	v2::PropertyIn<si::Angle>	set_angle		{ this, "/set-angle" };

  public:
	// Ctor
	explicit
	Flaps (std::string const& instance = {});

	void
	process (xf::Cycle const&) override;

  protected:
	// QWidget API
	void
	resizeEvent (QResizeEvent*) override;

	// QWidget API
	void
	paintEvent (QPaintEvent*) override;

  private:
	v2::PropertyObserver	_inputs_observer;
};

#endif
