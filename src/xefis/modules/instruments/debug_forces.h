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
#include <xefis/core/graphics.h>
#include <xefis/core/instrument.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/instrument/instrument_support.h>


namespace si = neutrino::si;


// TODO handle nans
class DebugForcesIO: public xf::ModuleIO
{
  public:
	/*
	 * Input
	 */

	xf::ModuleIn<si::Angle>			orientation_pitch				{ this, "orientation/pitch" };
	xf::ModuleIn<si::Angle>			orientation_roll				{ this, "orientation/roll" };
	xf::ModuleIn<si::Angle>			orientation_magnetic_heading	{ this, "orientation/magnetic-heading" };
	xf::ModuleIn<si::Acceleration>	measured_accel_x				{ this, "acceleration/x" };
	xf::ModuleIn<si::Acceleration>	measured_accel_y				{ this, "acceleration/y" };
	xf::ModuleIn<si::Acceleration>	measured_accel_z				{ this, "acceleration/z" };
	xf::ModuleIn<si::Acceleration>	centrifugal_accel_x				{ this, "centrifugal-acceleration/x" };
	xf::ModuleIn<si::Acceleration>	centrifugal_accel_y				{ this, "centrifugal-acceleration/y" };
	xf::ModuleIn<si::Acceleration>	centrifugal_accel_z				{ this, "centrifugal-acceleration/z" };
};


class DebugForces:
	public xf::Instrument<DebugForcesIO>,
	private xf::InstrumentSupport
{
  private:
	struct PaintingParams
	{
		std::optional<si::Angle>		orientation_pitch;
		std::optional<si::Angle>		orientation_roll;
		std::optional<si::Angle>		orientation_magnetic_heading;
		std::optional<si::Acceleration>	measured_accel_x;
		std::optional<si::Acceleration>	measured_accel_y;
		std::optional<si::Acceleration>	measured_accel_z;
		std::optional<si::Acceleration>	centrifugal_accel_x;
		std::optional<si::Acceleration>	centrifugal_accel_y;
		std::optional<si::Acceleration>	centrifugal_accel_z;
	};

  public:
	// Ctor
	explicit
	DebugForces (std::unique_ptr<DebugForcesIO>, xf::Graphics const&, std::string_view const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

	// Instrument API
	std::packaged_task<void()>
	paint (xf::PaintRequest) const override;

  private:
	void
	async_paint (xf::PaintRequest const&, PaintingParams const&) const;
};

#endif
