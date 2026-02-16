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
#include "radio_modules.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

RadioModules::RadioModules (xf::ProcessingLoop& loop, nu::Logger const& logger):
	_logger (logger),
	_loop (loop)
{ }

} // namespace sim1::aircraft
