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
#include "prandtl_tube_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/devices/prandtl_tube.h>
#include <xefis/support/ui/paint_helper.h>

// Standard:
#include <cstddef>


namespace xf::sim {

PrandtlTubeWidget::PrandtlTubeWidget (PrandtlTube& prandtl_tube):
	ObservationWidget (&prandtl_tube),
	_prandtl_tube (prandtl_tube)
{
	auto group = ObservationWidget::add_group (u8"Readings");
	group.add_observable ("Static pressure", _static_pressure);
	group.add_observable ("+ Dynamic pressure", _dynamic_pressure);
	group.add_observable ("= Total pressure", _total_pressure);
}


void
PrandtlTubeWidget::update_observed_values (rigid_body::Body const* planet_body)
{
	_static_pressure = std::format ("{:.3f}", _prandtl_tube.static_pressure());
	_dynamic_pressure = std::format ("{:.3f}", _prandtl_tube.total_pressure() - _prandtl_tube.static_pressure());
	_total_pressure = std::format ("{:.3f}", _prandtl_tube.total_pressure());

	ObservationWidget::update_observed_values (planet_body);
}

} // namespace xf::sim

