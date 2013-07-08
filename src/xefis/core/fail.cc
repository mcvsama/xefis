/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// System:
#include <unistd.h>
#include <signal.h>

// Standard:
#include <cstddef>
#include <iostream>
#include <iterator>
#include <vector>
#include <string>

// Local:
#include <xefis/config/version.h>
#include <xefis/utility/backtrace.h>
#include <xefis/core/services.h>

#include "fail.h"


namespace Xefis {

void
fail (int signum)
{
	std::vector<const char*> features = Xefis::Services::features();
	std::clog << "------------------------------------------------------------------------------------------------" << std::endl;
	std::clog << "Xefis died by signal." << std::endl << std::endl;
	std::clog << "  signal: " << signum << std::endl;
	std::clog << "  source info: " << std::endl;
	std::cout << "    commit: " << ::Xefis::Version::commit << std::endl;
	std::cout << "    branch: " << ::Xefis::Version::branch << std::endl;
	std::clog << "  features: ";
	std::copy (features.begin(), features.end(), std::ostream_iterator<const char*> (std::clog, " "));
	std::clog << std::endl;
	std::clog << "  backtrace:" << std::endl;
	Backtrace::clog();
	std::clog << "  CXXFLAGS: " << CXXFLAGS << std::endl << std::endl;
	// Force coredump if enabled:
	signal (signum, SIG_DFL);
	kill (getpid(), signum);
}

} // namespace Xefis

