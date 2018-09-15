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

// Standards:
#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>

// System:
#include <signal.h>
#include <locale.h>
#include <string.h>

// Qt:
#include <QtCore/QTextCodec>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/fail.h>
#include <xefis/core/xefis.h>
#include <xefis/utility/backtrace.h>


int main (int argc, char** argv, char**)
{
	signal (SIGILL, xf::fail);
	signal (SIGFPE, xf::fail);
	signal (SIGSEGV, xf::fail);
	signal (SIGHUP, [](int) { xf::g_hup_received.store (true); });

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
		{
			auto app = std::make_unique<xf::Xefis> (argc, argv);
			app->exec();
		}
	}
	catch (xf::Xefis::QuitInstruction)
	{
		return EXIT_SUCCESS;
	}
	catch (xf::Exception& e)
	{
		using namespace xf::exception_ops;

		std::cerr << "Fatal error: " << e << std::endl;
	}

	return EXIT_SUCCESS;
}

