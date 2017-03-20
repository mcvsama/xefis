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

// Boost:
#include <boost/variant.hpp>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "engine_torque.h"


EngineTorque::EngineTorque (std::string const& instance):
	Module (instance)
{ }


void
EngineTorque::process (x2::Cycle const&)
{
	struct TorqueComputer: public boost::static_visitor<>
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

	boost::apply_visitor (TorqueComputer (this), *setting_motor_efficiency);
}


void
EngineTorque::compute_torque (double motor_efficiency)
{
	if (input_engine_current)
		output_engine_torque = motor_efficiency * *input_engine_current / *setting_motor_kv;
	else
		output_engine_torque.set_nil();
}


void
EngineTorque::compute_torque (EfficiencyDatatable const& motor_efficiency)
{
	Optional<double> efficiency;

	if (input_engine_speed && (efficiency = motor_efficiency.value (*input_engine_speed)))
		compute_torque (*efficiency);
	else
		output_engine_torque.set_nil();
}

