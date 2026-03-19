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

void
Antenna::receive_signal (ReceivedSignal const& signal)
{
	if (_signal_reception_callback)
		_signal_reception_callback (signal);
}

} // namespace xf::sim
