/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__SIMULATOR_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__SIMULATOR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/simulation/rigid_body/impulse_solver.h>
#include <xefis/support/simulation/evolver.h>

// Neutrino:
#include <neutrino/logger.h>
#include <neutrino/noncopyable.h>

// Standard:
#include <cstddef>
#include <optional>


namespace xf {

/**
 * Rigid body + electrical simulator.
 */
class Simulator: public Noncopyable
{
  public:
	// Ctor
	explicit
	Simulator (rigid_body::System&, rigid_body::ImpulseSolver&, si::Time time_step, Logger const&);

	/**
	 * Return current simulation frame Δt.
	 */
	[[nodiscard]]
	si::Time
	time_step() const noexcept
		{ return _time_step; }

	/**
	 * Return integrated simulation time.
	 * This is the time how far the simulation has actually advanced and because Δt is not infinitely small, the result
	 * might be larger than real_time(), but not by more than frame Δt.
	 */
	[[nodiscard]]
	si::Time
	simulation_time() const noexcept
		{ return _evolver->simulation_time(); }

	/**
	 * Return rigid body system that's being simulated.
	 */
	[[nodiscard]]
	rigid_body::System const&
	rigid_body_system() const noexcept
		{ return _rigid_body_system; }

	/**
	 * Return rigid body system that's being simulated.
	 */
	[[nodiscard]]
	rigid_body::System&
	rigid_body_system() noexcept
		{ return _rigid_body_system; }

	/**
	 * Evolve the rigid body system by given dt. Multiple evolve() calls will be made on the System.
	 */
	void
	evolve (si::Time const simulation_time, si::Time const real_time_limit)
		{ _evolver->evolve (simulation_time, real_time_limit); }

  private:
	xf::Logger						_logger;
	si::Time const					_time_step;
	rigid_body::System&				_rigid_body_system;
	rigid_body::ImpulseSolver&		_rigid_body_solver;
	std::optional<xf::Evolver>		_evolver;
};

} // namespace xf

#endif

