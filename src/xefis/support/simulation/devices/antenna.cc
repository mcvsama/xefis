/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
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
#include "antenna.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>
#include <utility>


namespace xf::sim {

Antenna::Antenna (MassMoments<BodyCOM> const& mass_moments,
                  xf::AntennaModel const& antenna_model,
                  xf::AntennaSystem& antenna_system,
                  xf::Antenna::SignalReceptionCallback signal_reception_callback):
    Body (mass_moments),
    _antenna (antenna_model, antenna_system, std::move (signal_reception_callback))
{ }


Antenna::Antenna (MassMomentsAtArm<BodyCOM> const& mass_moments,
                  xf::AntennaModel const& antenna_model,
                  xf::AntennaSystem& antenna_system,
                  xf::Antenna::SignalReceptionCallback signal_reception_callback):
    Body (mass_moments),
    _antenna (antenna_model, antenna_system, std::move (signal_reception_callback))
{ }


void
Antenna::synchronize_antenna_placement() noexcept
{
	auto const placement = this->placement();
	auto const origin_placement_in_com = this->origin_placement_in_com();
	auto const origin_position_in_world = placement.rotate_translate_to_base (origin_placement_in_com.position());
	auto const origin_rotation_in_world = placement.body_rotation() * origin_placement_in_com.body_rotation();
	_antenna.set_placement (Placement<WorldSpace, BodyOrigin> (origin_position_in_world, origin_rotation_in_world));
}

} // namespace xf::sim
