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

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__COMMON__LINK__ADDRESSES_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__COMMON__LINK__ADDRESSES_H__INCLUDED

// Xefis:
#include <xefis/core/module.h>
#include <xefis/modules/comm/udp.h>


namespace sim1::global {

static inline auto hardware_to_flight_computer_address = UDP::Address { .host = "127.0.0.1", .port = 9991 };
static inline auto flight_computer_to_hardware_address = UDP::Address { .host = "127.0.0.1", .port = 9992 };

} // namespace sim1::global

#endif
