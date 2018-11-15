/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
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
#include <xefis/support/nature/constants.h>

// Local:
#include "engine.h"


namespace xf::sim {

ForceTorque<AirframeFrame>
Engine::forces (AtmosphereState<AirframeFrame> const& atm_body)
{
	// TODO engines have limited efficiency (0.8 * power?)
	// TODO engines cause drag
	// TODO engines cause p-factor: https://en.wikipedia.org/wiki/P-factor (one of the engines will have more p-factor than the other)
	// TODO engines efficiency depend on airspeed (air.wind and stuff)
	// TODO engines cause torque:
	//   TODO 1. when spooling up
	//   TODO 2. when working at constant speed (drag induced by blades)
	AtmosphereState<EngineFrame> const atm_engine { atm_body.air, this->base_to_body_rotation() * atm_body.wind };
	SpaceVector<si::Velocity, AirfoilSplineFrame> const planar_wind { atm_engine.wind[0], 0_mps, 0_mps }; // TODO AirfoilSplineFrame? What's happening here?
	auto const efficiency = 0.85;
	SpaceVector<si::Force, EngineFrame> const thrust_vec { _control.power / abs (planar_wind) * efficiency, 0_N, 0_N }; // P(t)=F⃗(t)⋅v⃗(t); F=P/v
	auto const thrust = thrust_vec * (atm_body.air.density / kStdAirDensity);
	Wrench const wrench {
		this->body_to_base_rotation() * thrust,
		{ 0_Nm, 0_Nm, 0_Nm },
		//SpaceVector<si::Torque, AirframeFrame> (math::zero),
		position(),
	};

	_control.temperature = 0_K; // TODO
	_control.angular_velocity = 0_radps; // TODO
	_control.thrust = abs (thrust);

	return resultant_force (wrench);
}

} // namespace xf::sim

