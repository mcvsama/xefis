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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__MACHINE_MANAGER_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__MACHINE_MANAGER_H__INCLUDED

// Local:
#include "machine.h"

// Xefis:
#include <xefis/app/xefis.h>
#include <xefis/config/all.h>
#include <xefis/core/machine_manager.h>

// Qt:
#include <QMainWindow>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

class MachineManager: public xf::MachineManager
{
  public:
	// Ctor
	MachineManager (xf::Xefis&);

	// xf::MachineManager API
	std::unique_ptr<xf::Machine>
	make_machine() override;

  private:
	void
	create_main_window();

	void
	restart_machine();

  private:
	std::optional<QMainWindow>	_main_window;
	std::optional<Machine>		_machine;
};

} // namespace sim1::aircraft

#endif

