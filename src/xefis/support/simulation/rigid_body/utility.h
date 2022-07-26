/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__UTILITY_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__UTILITY_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/body.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

/**
 * Make Earth body, oriented in ECEF frame of reference (X is 0°/0°, Y is 90°/0°, Z is north pole).
 */
[[nodiscard]]
std::unique_ptr<Body>
make_earth();

} // namespace xf::rigid_body

#endif

