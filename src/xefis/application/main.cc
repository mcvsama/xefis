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
#include <QtGui/QFontDatabase>

// Xefis:
#include <xefis/config/version.h>
#include <xefis/application/services.h>
#include <xefis/core/property_storage.h>
#include <xefis/utility/backtrace.h>
#include <widgets/efis.h>
#include <xefis/components/property_tree/property_tree.h>

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
			// Qt preparations:
			QTextCodec::setCodecForCStrings (QTextCodec::codecForName ("UTF-8"));
			QFontDatabase font_db;
			// Try to select best font:
			for (QString font: { "Black", "Bold", "BoldCondensed", "Condensed", "Light", "Medium", "Regular", "Thin" })
				QFontDatabase::addApplicationFont ("share/fonts/Roboto/Roboto-" + font + ".ttf");
			// Init Xefis modules:
			Xefis::PropertyStorage::initialize();

			for (auto font_family: { "Roboto", "Bitstream Vera Sans Mono", "Ubuntu Mono", "Droid Sans", "Trebuchet MS", "monospace" })
			{
				QFont font (font_family);
				QFontInfo font_info (font);
				if (font_info.exactMatch())
				{
					QApplication::setFont (font);
					break;
				}
			}
			// Now casting QString to std::string|const char* will yield UTF-8 encoded strings.
			// Also std::strings and const chars* are expected to be encoded in UTF-8.
			Xefis::PropertyTree* property_tree = new Xefis::PropertyTree (0);
			property_tree->show();
			EFIS* efis = new EFIS (nullptr);
			efis->show();
			efis->resize (610, 460);
			app->exec();
			delete efis;
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

