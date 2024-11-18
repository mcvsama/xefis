/* vim:ts=4
 *
 * Copyleft 2008…2023  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIM_1__GROUND_STATION__CONTROL_MACHINE__MODELS_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__GROUND_STATION__CONTROL_MACHINE__MODELS_H__INCLUDED

// Xefis:
#include <xefis/support/earth/air/standard_atmosphere.h>

// Standard:
#include <cstddef>


namespace sim1::ground_station::control_machine {

class Models
{
  public:
	xf::StandardAtmosphere standard_atmosphere;
};

} // namespace sim1::ground_station::control_machine

#endif

