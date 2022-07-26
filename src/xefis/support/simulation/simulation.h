/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__SIMULATION_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__SIMULATION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/force_moments.h>

// Neutrino:
#include <neutrino/logger.h>

// Standard:
#include <cstddef>
#include <functional>


namespace xf {

/**
 * Generic simulation. Calls provided evolution function with configured Δt.
 */
class Simulation
{
  public:
	// Evolution function called on each simulation frame:
	using Evolve = std::function<void (si::Time dt)>;

  public:
	/**
	 * Ctor
	 *
	 * \param	evolve
	 *			Evolution function called for each simulation frame.
	 *			Must not be nullptr.
	 */
	explicit
	Simulation (si::Frequency world_frequency, Logger const&, Evolve);

	/**
	 * Return current simulation frame Δt.
	 */
	[[nodiscard]]
	si::Time
	frame_dt() const noexcept
		{ return _frame_dt; }

	/**
	 * Set new simulation frame Δt.
	 */
	void
	set_frame_dt (si::Time const dt) noexcept
		{ _frame_dt = dt; }

	/**
	 * Return integrated simulation time.
	 * This is the time how far the simulation has actually advanced and because Δt is not infinitely small, the result
	 * might be larger than real_time(), but not by more than frame Δt.
	 */
	[[nodiscard]]
	si::Time
	time() const noexcept
		{ return _simulation_time; }

	/**
	 * Return integrated real time (sum of Δt passed to evolve()).
	 */
	[[nodiscard]]
	si::Time
	real_time() const noexcept
		{ return _real_time; }

	/**
	 * Evolve the rigid body system by given dt. Multiple evolve() calls will be made on the System.
	 */
	void
	evolve (si::Time dt, si::Time real_time_limit);

  private:
	xf::Logger	_logger;
	si::Time	_real_time			{ 0_s };
	si::Time	_simulation_time	{ 0_s };
	si::Time	_frame_dt;
	Evolve		_evolve;
};

} // namespace xf

#endif

