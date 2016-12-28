/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/compatibility_v1_v2.h>
#include <xefis/modules/helpers/mixer.h>
#include <xefis/modules/systems/adc.h>
#include <xefis/modules/systems/afcs.h>
#include <xefis/modules/systems/flaps_control.h>
#include <xefis/support/airframe/airframe.h>

// Local:
#include "cthulhu.h"


// TODO temp
class DummyModule: public x2::Module
{
  public:
	x2::PropertyOut<int64_t>	output_int	{ this, "/output/int" };
	x2::PropertyIn<int64_t>		input_int	{ this, "/input/int" };

  public:
	explicit DummyModule()
	{ }

	virtual void
	process (x2::Cycle const&) override
	{
		if (output_int.is_nil())
			output_int = 0;

		if (input_int.valid())
			output_int = *input_int + 1;

		//std::cout << "DummyModule::process(" << input_int.value_or (-1) << ") -> " << *output_int << "\n";
	}
};


// TODO temp
class TempModule: public x2::Module
{
  public:
	x2::PropertyOut<int64_t>	output_int		{ this, "/output/int" };
	x2::PropertyIn<int64_t>		input_int		{ this, "/input/int" };
	x2::PropertyIn<Temperature>	temperature_in	{ this, "/input/temperature" };

  public:
	explicit TempModule()
	{ }

	virtual void
	process (x2::Cycle const&) override
	{
		if (output_int.is_nil())
			output_int = 0;

		if (input_int.valid())
			output_int = *input_int + 1;

		//std::cout << "TempModule::process(" << input_int.value_or (-1) << ") -> " << *output_int << "\n";
	}
};


TempModule* g_temp_module = nullptr;


class MyLoop: public x2::ProcessingLoop
{
  public:
	using ProcessingLoop::ProcessingLoop;

	void
	execute_cycle() override
	{
		ProcessingLoop::execute_cycle();
		std::cout << "CYCLE\n";

		if (g_temp_module && g_temp_module->temperature_in.valid())
			std::cout << "TEMP: " << *g_temp_module->temperature_in << "\n";
	}
};


Cthulhu::Cthulhu (xf::Xefis* xefis):
	Machine (xefis)
{
	xf::Airframe airframe (xefis, QDomElement()); // XXX

	auto* loop = make_processing_loop<MyLoop> (100_Hz);
	auto* flaps_control = loop->load_module<FlapsControl> (airframe);
	auto* adc = loop->load_module<AirDataComputer> (&airframe);
	auto* afcs = loop->load_module<AFCS>();
	auto* dummy_module = loop->load_module<DummyModule>();
	auto* temp_module = loop->load_module<TempModule>();
	auto* mixer = loop->load_module<Mixer<si::Angle>> ("mixer");
	g_temp_module = temp_module;

	//flaps_control->setting_angular_velocity = 2.5_deg / 1_s;
	//flaps_control->setting_control_extents = { 0.0, 0.5 };

	dummy_module->output_int >> temp_module->input_int;
	dummy_module->input_int << temp_module->output_int;

	temp_module->temperature_in << xf::Property<Temperature> (xf::PropertyPath ("/sensors/air-temperature/total"));

	loop->start();
}

