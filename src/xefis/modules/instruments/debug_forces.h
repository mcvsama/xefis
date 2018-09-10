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

#ifndef XEFIS__MODULES__INSTRUMENTS__DEBUG_FORCES_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__DEBUG_FORCES_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/core/property.h>
#include <xefis/support/instrument/instrument_support.h>


class DebugForcesIO: public xf::ModuleIO
{
  public:
	/*
	 * Input
	 */

	xf::PropertyIn<si::Angle>			orientation_pitch				{ this, "orientation/pitch" };
	xf::PropertyIn<si::Angle>			orientation_roll				{ this, "orientation/roll" };
	xf::PropertyIn<si::Angle>			orientation_magnetic_heading	{ this, "orientation/magnetic-heading" };
	xf::PropertyIn<si::Acceleration>	measured_accel_x				{ this, "acceleration/x" };
	xf::PropertyIn<si::Acceleration>	measured_accel_y				{ this, "acceleration/y" };
	xf::PropertyIn<si::Acceleration>	measured_accel_z				{ this, "acceleration/z" };
	xf::PropertyIn<si::Acceleration>	centrifugal_accel_x				{ this, "centrifugal-acceleration/x" };
	xf::PropertyIn<si::Acceleration>	centrifugal_accel_y				{ this, "centrifugal-acceleration/y" };
	xf::PropertyIn<si::Acceleration>	centrifugal_accel_z				{ this, "centrifugal-acceleration/z" };
};


class DebugForces:
	public xf::Instrument<DebugForcesIO>,
	private xf::InstrumentSupport
{
  public:
	// Ctor
	explicit
	DebugForces (std::unique_ptr<DebugForcesIO>, std::string_view const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

	// Instrument API
	void
	paint (xf::PaintRequest&) const override;
};

#endif
