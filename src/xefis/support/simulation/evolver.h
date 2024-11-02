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

/**
 * Helper for evolving simulations with configured time step.
 * With configured time step 1_ms if we call evolve (1_s),
 * it will cause evolution of 1000 frames.
 */
class Evolver
{
  public:
	// Evolution function called on each simulation frame:
	using Evolve = std::function<void (si::Time frame_duration)>;

	struct EvolutionResult
	{
		si::Time	real_time_taken;
		std::size_t	evolved_frames;
	};

  public:
	/**
	 * Ctor
	 *
	 * \param	evolve
	 *			Evolution function called for each simulation frame.
	 *			Must not be nullptr.
	 */
	explicit
	Evolver (si::Time frame_duration, Logger const&, Evolve);

	/**
	 * Return current simulation frame Δt.
	 */
	[[nodiscard]]
	si::Time
	frame_duration() const noexcept
		{ return _frame_duration; }

	/**
	 * Set new simulation frame Δt.
	 */
	void
	set_frame_duration (si::Time const dt) noexcept
		{ _frame_duration = dt; }

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
	 * Evolve the rigid body system by given simulation time. Multiple evolve() calls will be made on the System.
	 */
	EvolutionResult
	evolve (si::Time duration);

	/**
	 * Evolve the rigid body system given number of steps (frames).
	 */
	EvolutionResult
	evolve (std::size_t frames);

	/**
	 * Return performance factor. It tells how much simulation time has passed per real time.
	 * 1.0 or more is desired, values below 1.0 mean that the system can't simulate in real time.
	 */
	[[nodiscard]]
	float
	performance() const noexcept
		{ return _performance; }

  private:
	xf::Logger	_logger;
	si::Time	_frame_duration;
	Evolve		_evolve;
	si::Time	_target_time		{ 0_s };
	si::Time	_simulation_time	{ 0_s };
	float		_performance		{ 1.0 };
};

} // namespace xf

#endif

