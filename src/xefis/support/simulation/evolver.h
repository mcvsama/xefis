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

#ifndef XEFIS__SUPPORT__SIMULATION__EVOLVER_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__EVOLVER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/logger.h>

// Standard:
#include <cstddef>
#include <functional>


namespace xf {

struct EvolveParameters
{
	si::Time	time_step;
	si::Time	real_time_limit;
};


/**
 * Helper for evolving simulations with configured time step.
 * With configured time step 1_ms if we call evolve (1_s),
 * it will cause evolution of 1000 frames.
 */
class Evolver
{
  public:
	// Evolution function called on each simulation frame:
	using Evolve = std::function<void (si::Time time_step)>;

  public:
	/**
	 * Ctor
	 *
	 * \param	evolve
	 *			Evolution function called for each simulation frame.
	 *			Must not be nullptr.
	 */
	explicit
	Evolver (si::Time time_step, Logger const&, Evolve);

	/**
	 * Return current simulation frame Δt.
	 */
	[[nodiscard]]
	si::Time
	time_step() const noexcept
		{ return _time_step; }

	/**
	 * Set new simulation frame Δt.
	 */
	void
	set_time_step (si::Time const dt) noexcept
		{ _time_step = dt; }

	/**
	 * Return integrated simulation time.
	 * This is the time how far the simulation has actually advanced and because Δt is not infinitely small, the result
	 * might be larger than real_time(), but not by more than frame Δt.
	 */
	[[nodiscard]]
	si::Time
	simulation_time() const noexcept
		{ return _simulation_time; }

	/**
	 * Evolve the rigid body system by given dt. Multiple evolve() calls will be made on the System.
	 */
	void
	evolve (si::Time simulation_time, si::Time real_time_limit);

  private:
	xf::Logger	_logger;
	si::Time	_time_step;
	Evolve		_evolve;
	si::Time	_real_time			{ 0_s };
	si::Time	_simulation_time	{ 0_s };
};

} // namespace xf

#endif

