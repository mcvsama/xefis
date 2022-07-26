/* vim:ts=4
 *
 * Copyleft 2021  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__SOCKETS__COMMON_H__INCLUDED
#define XEFIS__CORE__SOCKETS__COMMON_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/stdexcept.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Helper type that indicates Nil values for sockets.
 */
class Nil
{ };


/**
 * Helper type that's used to reset data source for a socket.
 */
class NoDataSource
{ };


/**
 * Global nil object that when compared to a nil socket, gives true.
 */
static constexpr Nil nil;


/**
 * Global NoDataSource object.
 */
static constexpr NoDataSource no_data_source;

} // namespace xf

#endif

