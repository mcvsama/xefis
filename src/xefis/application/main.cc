/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <xefis/config/version.h>
#include <xefis/application/services.h>
#include <xefis/core/property_storage.h>
#include <xefis/utility/backtrace.h>
#include <input/flight_gear.h>
#include <instruments/efis.h>
#include <xefis/components/property_tree/property_storage_widget.h>

// Local:
#include "fail.h"


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

			Xefis::PropertyStorageWidget* property_storage_widget = new Xefis::PropertyStorageWidget (Xefis::PropertyStorage::root(), nullptr);
			property_storage_widget->show();

			Xefis::Input* input = new FlightGearInput();

			Xefis::Instrument* instrument = new EFIS (nullptr);
			instrument->show();
			instrument->resize (610, 460);

			app->exec();

			delete instrument;
			delete input;

			Xefis::Services::deinitialize();

			delete app;
		}
	}
	catch (...)
	{
		Backtrace::clog();
		throw;
	}

	return EXIT_SUCCESS;
}

