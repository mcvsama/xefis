/* vim:ts=4
 *
 * Copyleft 2018  Michał Gawron
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
#include <xefis/core/clock.h>
#include <xefis/core/machine_manager.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/simulation/rigid_body/impulse_solver.h>
#include <xefis/support/simulation/evolver.h>

// Neutrino:
#include <neutrino/logger.h>
#include <neutrino/noncopyable.h>

// Lib:
#include <boost/circular_buffer.hpp>

// Standard:
#include <cstddef>
#include <list>
#include <optional>
#include <ranges>
#include <utility>


namespace xf {

class Machine;


/**
 * Rigid body + electrical simulator + some helper stuff like list of machines
 * used in a simulation.
 */
class Simulator: public nu::Noncopyable
{
	static constexpr std::size_t kMaxEvolutionDetailsHistory { 5000 };

  public:
	using MachineManagers = std::list<BasicMachineManager*>;
	using Clocks = std::list<SimulatedClock*>;
	using IterationsRunHistory = boost::circular_buffer<double>;
	using RealTimeTakenHistory = boost::circular_buffer<si::Time>;
	using IterationsRunHistoryRange = std::ranges::subrange<IterationsRunHistory::const_iterator>;
	using RealTimeTakenHistoryRange = std::ranges::subrange<RealTimeTakenHistory::const_iterator>;

  public:
	// Ctor
	explicit
	Simulator (rigid_body::System&, rigid_body::ImpulseSolver&, si::Time initial_simulation_time, si::Time frame_duration, nu::Logger const&);

	/**
	 * Set additional evolution callback.
	 */
	void
	set_additional_evolution_callback (Evolver::Evolve evolve)
		{ _additional_evolve = std::move (evolve); }

	/**
	 * Return current simulation frame Δt.
	 */
	[[nodiscard]]
	si::Time
	frame_duration() const noexcept
		{ return _evolver->frame_duration(); }

	/**
	 * Return virtual simulation time.
	 */
	[[nodiscard]]
	si::Time
	simulation_time() const noexcept
		{ return _evolver->simulation_time(); }

	/**
	 * Return integrated virtual elapsed time, that is simulated time elapsed since the start of the simulation.
	 * This is the time how far the simulation has actually advanced and because Δt is not infinitely small, the result might be larger than real_time(), but
	 * not by more than frame Δt.
	 */
	[[nodiscard]]
	si::Time
	elapsed_time() const noexcept
		{ return _evolver->elapsed_time(); }

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
	 * Return a subrange of managed clocks.
	 */
	std::ranges::subrange<Clocks::iterator>
	clocks() noexcept
		{ return { _clocks.begin(), _clocks.end() }; }

	/**
	 * Return a subrange of managed clocks.
	 */
	std::ranges::subrange<Clocks::const_iterator>
	clocks() const noexcept
		{ return { _clocks.begin(), _clocks.end() }; }

	/**
	 * Return a subrange of managed machines.
	 */
	std::ranges::subrange<MachineManagers::iterator>
	machine_managers() noexcept
		{ return { _machine_managers.begin(), _machine_managers.end() }; }

	/**
	 * Return a subrange of managed machines.
	 */
	std::ranges::subrange<MachineManagers::const_iterator>
	machine_managers() const noexcept
		{ return { _machine_managers.begin(), _machine_managers.end() }; }

	/**
	 * Add SimulatedClock to manage.
	 * The clock will be advanced along with the simulation.
	 */
	void
	register_clock (SimulatedClock& clock)
		{ _clocks.push_back (&clock); }

	/**
	 * Add MachineManager to manage.
	 * Machine gets started/stopped and stepped along with the simulation.
	 */
	void
	register_machine_manager (BasicMachineManager& machine_manager)
		{ _machine_managers.push_back (&machine_manager); }

	/**
	 * Evolve the rigid body system by given Δt. Multiple evolve() calls will be made on the System.
	 */
	void
	evolve (si::Time const duration)
		{ _evolver->evolve (duration); }

	/**
	 * Evolve the rigid body system given number of steps (frames).
	 */
	void
	evolve (std::size_t const frames)
		{ _evolver->evolve (frames); }

	/**
	 * Return Evolver::performance().
	 */
	[[nodiscard]]
	float
	performance() const noexcept
		{ return _evolver->performance(); }

	[[nodiscard]]
	IterationsRunHistoryRange
	iterations_run_history() const noexcept
		{ return { _iterations_run_history.begin(), _iterations_run_history.end() }; }

	[[nodiscard]]
	RealTimeTakenHistoryRange
	real_time_taken_history() const noexcept
		{ return { _real_time_taken_history.begin(), _real_time_taken_history.end() }; }

  private:
	nu::Logger					_logger;
	rigid_body::System&			_rigid_body_system;
	rigid_body::ImpulseSolver&	_rigid_body_solver;
	std::optional<Evolver>		_evolver;
	Evolver::Evolve				_additional_evolve;
	IterationsRunHistory		_iterations_run_history		{ kMaxEvolutionDetailsHistory };
	RealTimeTakenHistory		_real_time_taken_history	{ kMaxEvolutionDetailsHistory };
	MachineManagers				_machine_managers;
	Clocks						_clocks;
};

} // namespace xf

#endif
