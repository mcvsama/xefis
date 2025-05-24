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

#ifndef XEFIS__SUPPORT__CORE__SINGLE_MACHINE_MANAGER_H__INCLUDED
#define XEFIS__SUPPORT__CORE__SINGLE_MACHINE_MANAGER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine_manager.h>

// Standard:
#include <cstddef>


namespace xf {

class Xefis;
class ConfiguratorWidget;


class SingleMachineManager: public MachineManager
{
  public:
	// Ctor
	SingleMachineManager (std::unique_ptr<Machine> machine, Xefis& xefis);

	// MachineManager API
	Machine&
	machine() override
		{ return *_machine; }

  private:
	std::unique_ptr<Machine>			_machine;
	std::unique_ptr<ConfiguratorWidget>	_configurator_widget;
};

} // namespace xf

#endif

