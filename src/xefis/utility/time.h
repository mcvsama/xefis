/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__UTILITY__TIME_H__INCLUDED
#define XEFIS__UTILITY__TIME_H__INCLUDED

// Standard:
#include <cstddef>

// System:
#include <time.h>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

void
sleep (Time time)
{
	struct timespec ts;
	ts.tv_sec = static_cast<decltype (ts.tv_sec)> (time.s());
	ts.tv_nsec = (time - ts.tv_sec * 1_s).ns();

	do {
		if (nanosleep (&ts, &ts) == -1)
			if (errno == EINTR)
				continue;
	} while (false);
}

} // namespace Xefis

#endif

