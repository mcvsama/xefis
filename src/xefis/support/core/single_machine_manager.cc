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

// Local:
#include "single_machine_manager.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/components/configurator/configurator_widget.h>
#include <xefis/core/xefis.h>

// Standard:
#include <cstddef>


namespace xf {

SingleMachineManager::SingleMachineManager (std::unique_ptr<Machine> machine, Xefis& xefis):
	MachineManager (xefis),
	_machine (std::move (machine))
{
	if (!_machine)
		throw InvalidArgument ("SingleMachineManager: machine must be non-null");

	_configurator_widget = std::make_unique<ConfiguratorWidget> (*_machine, nullptr);
}

} // namespace xf

