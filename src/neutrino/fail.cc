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

// System:
#include <unistd.h>
#include <signal.h>

// Standard:
#include <cstddef>
#include <iostream>
#include <iterator>
#include <vector>
#include <string>

// Neutrino:
#include <neutrino/core/version.h>
#include <neutrino/backtrace.h>

// Local:
#include "fail.h"


namespace neutrino {

std::atomic<bool> g_hup_received { false };


void
fail (int signum)
{
	using std::endl;

	std::clog << endl;
	std::clog << "------------------------------------------------------------------------------------------------" << endl;
	std::clog << "Program died by a signal." << endl << endl;
	std::clog << "       signal: " << signum << endl;
	std::clog << "  source info: " << endl;
	std::cout << "       commit: " << ::neutrino::version::commit << endl;
	std::cout << "       branch: " << ::neutrino::version::branch << endl;
	std::clog << "    backtrace:" << endl;
	std::clog << backtrace().resolve_sources() << endl;
	std::clog << "     CXXFLAGS: " << CXXFLAGS << endl << endl;
	// Force coredump if enabled:
	signal (signum, SIG_DFL);
	kill (getpid(), signum);
}

} // namespace neutrino

