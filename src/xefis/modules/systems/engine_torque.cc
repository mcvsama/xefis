/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>
#include <variant>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "engine_torque.h"


EngineTorque::EngineTorque (std::unique_ptr<EngineTorqueIO> module_io, std::string const& instance):
	Module (std::move (module_io), instance)
{ }


void
EngineTorque::process (v2::Cycle const&)
{
	struct TorqueComputer
	{
		explicit
		TorqueComputer (EngineTorque* module):
			_module (module)
		{ }

		void operator() (double motor_efficiency) const
		{
			_module->compute_torque (motor_efficiency);
		}

		void operator() (EfficiencyDatatable const& datatable) const
		{
			_module->compute_torque (datatable);
		}

	  private:
		EngineTorque* _module;
	};

	std::visit (TorqueComputer (this), *io.motor_efficiency);
}


void
EngineTorque::compute_torque (double motor_efficiency)
{
	if (io.engine_current)
		io.engine_torque = motor_efficiency * *io.engine_current / *io.motor_kv;
	else
		io.engine_torque.set_nil();
}


void
EngineTorque::compute_torque (EfficiencyDatatable const& motor_efficiency)
{
	std::optional<double> efficiency;

	if (io.engine_speed && (efficiency = motor_efficiency.value (*io.engine_speed)))
		compute_torque (*efficiency);
	else
		io.engine_torque.set_nil();
}

