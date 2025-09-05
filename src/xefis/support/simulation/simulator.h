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
#include <xefis/core/machine_manager.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/simulation/rigid_body/impulse_solver.h>
#include <xefis/support/simulation/evolver.h>

// Neutrino:
#include <neutrino/logger.h>
#include <neutrino/noncopyable.h>

// Standard:
#include <cstddef>
#include <optional>
#include <ranges>
#include <vector>


namespace xf {

class Machine;


/**
 * Rigid body + electrical simulator + some helper stuff like list of machines
 * used in a simulation.
 */
class Simulator: public nu::Noncopyable
{
  public:
	using MachineManagersVector = std::vector<BasicMachineManager*>;

  public:
	// Ctor
	explicit
	Simulator (rigid_body::System&, rigid_body::ImpulseSolver&, si::Time initial_simulation_time, si::Time frame_duration, nu::Logger const&);

	/**
	 * Set additional evolution callback.
	 */
	void
	set_additional_evolution_callback (Evolver::Evolve const evolve)
		{ _additional_evolve = evolve; }

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
	 * Set Machine to manage (start/stop and step along with the simulation).
	 * Can be nullptr to disable machine managing.
	 */
	void
	add_machine_manager (BasicMachineManager& machine_manager)
		{ _machine_managers.push_back (&machine_manager); }

	/**
	 * Return a subrange of managed machines.
	 */
	std::ranges::subrange<MachineManagersVector::iterator>
	machine_managers() noexcept
		{ return { _machine_managers.begin(), _machine_managers.end() }; }

	/**
	 * Return a subrange of managed machines.
	 */
	std::ranges::subrange<MachineManagersVector::const_iterator>
	machine_managers() const noexcept
		{ return { _machine_managers.begin(), _machine_managers.end() }; }

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

  private:
	nu::Logger					_logger;
	rigid_body::System&			_rigid_body_system;
	rigid_body::ImpulseSolver&	_rigid_body_solver;
	std::optional<Evolver>		_evolver;
	Evolver::Evolve				_additional_evolve;
	MachineManagersVector		_machine_managers;
};

} // namespace xf

#endif

