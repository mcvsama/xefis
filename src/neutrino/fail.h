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

#ifndef NEUTRINO__FAIL_H__INCLUDED
#define NEUTRINO__FAIL_H__INCLUDED

// Standard:
#include <atomic>
#include <cstddef>


namespace neutrino {

/**
 * Set to true when HUP signal is received.
 */
extern std::atomic<bool> g_hup_received;


/**
 * Called as a UNIX signal handler.
 * Prints stacktrace and other useful information on std::clog.
 */
extern void
fail (int signum);

} // namespace neutrino

#endif

