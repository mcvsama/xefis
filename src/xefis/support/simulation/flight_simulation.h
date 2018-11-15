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

#ifndef XEFIS__SUPPORT__SIMULATION__FLIGHT_SIMULATION_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__FLIGHT_SIMULATION_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>

// Lib:
#include <lib/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/logger.h>
#include <xefis/support/simulation/n_body/body.h>
#include <xefis/support/simulation/airframe.h>
#include <xefis/support/simulation/atmosphere.h>


namespace xf::sim {

class FlightSimulation
{
	static constexpr si::Force				kMaxForce			{ 1000_N };
	static constexpr si::Torque				kMaxTorque			{ 1000_Nm };
	static constexpr si::Velocity			kMaxVelocity		{ 1000_mps };
	static constexpr si::AngularVelocity	kMaxAngularVelocity	{ 100_radps };

  public:
	// Ctor
	explicit
	FlightSimulation (Airframe&&, si::Frequency update_frequency, Logger const&);

	void
	evolve (si::Time dt, si::Time dt_limit);

	/**
	 * Airframe reference.
	 */
	[[nodiscard]]
	Airframe const&
	airframe() const noexcept
		{ return _airframe; }

	/**
	 * Return all forces acting on the body, except gravitational ones.
	 */
	[[nodiscard]]
	ForceTorque<ECEFFrame> const&
	airframe_forces() const noexcept
		{ return _airframe_forces; }

	/**
	 * Reference to the Atmosphere object used in simulation.
	 */
	Atmosphere const&
	atmosphere() const noexcept
		{ return _atmosphere; }

	/**
	 * Return AtmosphereState at given point relative to the body center-of-mass and in body frame of reference.
	 * Note the wind will be a relative wind to the airframe.
	 */
	AtmosphereState<AirframeFrame>
	complete_atmosphere_state_at (SpaceVector<si::Length, AirframeFrame> const com_relative_part_position) const
		{ return _airframe.complete_atmosphere_state_at (com_relative_part_position, _atmosphere); }

  private:
	static constexpr std::size_t	kAirframeIndex		{ 0 };
	static constexpr std::size_t	kEarthIndex			{ 1 };
	xf::Logger						_logger;
	si::Time						_real_time			{ 0_s };
	si::Time						_simulation_time	{ 0_s };
	si::Time						_frame_dt;
	Atmosphere						_atmosphere;
	Airframe						_airframe;
	Body							_earth;
	std::array<Body*, 2>			_bodies				{ &_airframe, &_earth };
	ForceTorque<ECEFFrame>			_airframe_forces;
};

} // namespace xf::sim

#endif

