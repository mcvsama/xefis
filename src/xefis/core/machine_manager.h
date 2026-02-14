/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__MACHINE_MANAGER_H__INCLUDED
#define XEFIS__CORE__MACHINE_MANAGER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/components/configurator/configurator_widget.h>
#include <xefis/core/machine.h>
#include <xefis/core/xefis.h>

// Neutrino:
#include <neutrino/noncopyable.h>

// Standard:
#include <cstddef>
#include <concepts>
#include <functional>
#include <memory>


namespace xf {

class Xefis;


class BasicMachineManager: private nu::Noncopyable
{
  public:
	// Ctor
	BasicMachineManager (std::u8string_view const name):
		_name (name)
	{ }

	// Dtor
	virtual
	~BasicMachineManager() = default;

	std::u8string const&
	name() const
		{ return _name; }

	virtual void
	restart() = 0;

	[[nodiscard]]
	virtual Machine*
	machine() = 0;

	virtual void
	kill() = 0;

  private:
	std::u8string _name;
};


/**
 * Manages machine.
 */
template<std::derived_from<Machine> ConcreteMachine>
	class MachineManager: public BasicMachineManager
	{
	  public:
		using MakeMachineFunction = std::function<std::unique_ptr<ConcreteMachine>()>;

	  public:
		/**
		 * Machine is not created until first restart() is called.
		 */
		explicit
		MachineManager (std::u8string_view const name):
			MachineManager (name, std::in_place)
		{ }

		/**
		 * Machine is not created until first restart() is called.
		 */
		explicit
		MachineManager (std::u8string_view const name, MachineManager::MakeMachineFunction const make_machine):
			BasicMachineManager (name),
			_make_machine (make_machine)
		{ }

		// Dtor
		virtual
		~MachineManager() = default;

		/**
		 * Return the managed machine or nullptr if not running.
		 */
		[[nodiscard]]
		ConcreteMachine*
		machine() override
			{ return _machine.get(); }

		/**
		 * Start or restart the machine.
		 * Only available if Machine can be default-constructed.
		 */
		void
		restart() override
			{ _machine = _make_machine(); }

		void
		kill() override
			{ _machine.reset(); }

	  private:
		// TODO into a single struct as they're coupled:
		std::function<std::unique_ptr<ConcreteMachine>()>	_make_machine;
		std::unique_ptr<ConcreteMachine>					_machine;
	};

} // namespace xf

#endif
