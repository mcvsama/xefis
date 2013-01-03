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

// Standards:
#include <cstddef>
#include <cstdlib>
#include <iostream>

// System:
#include <signal.h>
#include <locale.h>
#include <string.h>

// Qt:
#include <QtCore/QTextCodec>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/services.h>
#include <xefis/core/property_storage.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/config_reader.h>
#include <xefis/utility/backtrace.h>

// Local:
#include "fail.h"


void
log_exception (Xefis::Exception const& e)
{
	std::cerr << "Error: " << e.what() << std::endl;
	std::cerr << e.backtrace() << std::endl;
	if (e.inner())
		log_exception (*e.inner());
}


int main (int argc, char** argv, char**)
{
	signal (SIGILL, Xefis::fail);
	signal (SIGFPE, Xefis::fail);
	signal (SIGSEGV, Xefis::fail);

	setenv ("LC_ALL", "POSIX", 1);
	setlocale (LC_ALL, "POSIX");

	try {
		if (argc == 2 && (strcmp (argv[1], "-v") == 0 || strcmp (argv[1], "--version") == 0))
		{
			std::cout << "Xefis" << std::endl;
			std::cout << "Commit: " << Xefis::Version::commit << std::endl;
			std::cout << "Branch: " << Xefis::Version::branch << std::endl;
			std::clog << "Features: ";
			std::vector<const char*> features = Xefis::Services::features();
			std::copy (features.begin(), features.end(), std::ostream_iterator<const char*> (std::clog, " "));
			std::clog << std::endl;
		}
		else
		{
			QApplication* app = new QApplication (argc, argv);
			// Casting QString to std::string|const char* should yield UTF-8 encoded strings.
			// Also encode std::strings and const chars* in UTF-8:
			QTextCodec::setCodecForCStrings (QTextCodec::codecForName ("UTF-8"));
			// Init services:
			Xefis::Services::initialize();
			// Init Xefis modules:
			Xefis::PropertyStorage::initialize();

			Xefis::ModuleManager* module_manager = new Xefis::ModuleManager();
			Xefis::ConfigReader config_reader (module_manager);
			config_reader.load ("xefis-config.xml");

			app->exec();

			delete module_manager;

			Xefis::Services::deinitialize();

			delete app;
		}
	}
	catch (Xefis::Exception& e)
	{
		log_exception (e);
	}
	catch (...)
	{
		Backtrace::clog();
		throw;
	}

	return EXIT_SUCCESS;
}

