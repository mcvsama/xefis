/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
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
#include "system.h"

// Xefis:
#include <xefis/app/xefis.h>
#include <xefis/config/all.h>

// System:
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

// Standard:
#include <cstddef>


namespace xf {

System::System (Logger const& logger):
	_logger (logger.with_scope ("<system>"))
{
	_logger << "Creating System object" << std::endl;
}


System::~System()
{
	_logger << "Destroying System" << std::endl;
}


bool
System::set_clock (si::Time const unix_time)
{
	::timeval tv = { static_cast<time_t> (unix_time.in<si::Second>()), 0 };
	if (::settimeofday (&tv, nullptr) < 0)
	{
		_logger << "Could not setup system time: settimeofday() failed with error '" << strerror (errno) << "'; "
				<< "ensure that Xefis executable has CAP_SYS_TIME capability set with 'setcap cap_sys_time+ep path-to-xefis-executable'" << std::endl;
		return false;
	}
	else
		return true;
}

} // namespace xf

