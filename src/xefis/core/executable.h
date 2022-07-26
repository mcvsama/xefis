/* vim:ts=4
 *
 * Copyleft 2012…2022  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__APPLICATION_H__INCLUDED
#define XEFIS__CORE__APPLICATION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/backtrace.h>
#include <neutrino/fail.h>

// Qt:
#include <QtCore/QTextCodec>

// System:
#include <signal.h>
#include <locale.h>
#include <string.h>

// Standard:
#include <cstddef>
#include <functional>
#include <iostream>


namespace xf {

class QuitInstruction { };


inline int
setup_xefis_executable (int argc, char** argv, std::function<void()> run_app_function)
{
	signal (SIGILL, neutrino::fail);
	signal (SIGFPE, neutrino::fail);
	signal (SIGSEGV, neutrino::fail);
	signal (SIGHUP, [](int) { neutrino::g_hup_received.store (true); });

	setenv ("LC_ALL", "POSIX", 1);
	setlocale (LC_ALL, "POSIX");

	try {
		if (argc == 2 && (strcmp (argv[1], "-v") == 0 || strcmp (argv[1], "--version") == 0))
		{
			std::cout << "Xefis" << std::endl;
			std::cout << "Commit: " << xf::version::commit << std::endl;
			std::cout << "Branch: " << xf::version::branch << std::endl;
			std::clog << "CXXFLAGS: " << CXXFLAGS << std::endl;
			std::clog << std::endl;
		}
		else
			run_app_function();
	}
	catch (QuitInstruction)
	{
		return EXIT_SUCCESS;
	}
	catch (xf::Exception& e)
	{
		using namespace xf::exception_ops;

		std::cerr << "Fatal error: " << e << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

} // namespace xf

#endif

