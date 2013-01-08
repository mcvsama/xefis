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

// Standard:
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Qt:
#include <QtCore/QTextCodec>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/services.h>
#include <xefis/core/property_storage.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/config_reader.h>

// Local:
#include "application.h"


namespace Xefis {

Application* Application::_application = nullptr;


Application::Application (int argc, char** argv):
	QApplication (argc, argv)
{
	// Casting QString to std::string|const char* should yield UTF-8 encoded strings.
	// Also encode std::strings and const chars* in UTF-8:
	QTextCodec::setCodecForLocale (QTextCodec::codecForName ("UTF-8"));
	// Init services:
	Xefis::Services::initialize();
	// Init Xefis modules:
	Xefis::PropertyStorage::initialize();

	_module_manager = new Xefis::ModuleManager();
	_config_reader = new Xefis::ConfigReader (_module_manager);
	_config_reader->load ("xefis-config.xml");
}


Application::~Application()
{
	delete _config_reader;
	delete _module_manager;
	Xefis::Services::deinitialize();
}


void
Application::quit()
{
	closeAllWindows();
}


void
Application::data_update()
{
	postEvent (this, new DataUpdateEvent());
}


bool
Application::event (QEvent* event)
{
	if (dynamic_cast<DataUpdateEvent*> (event))
	{
		_module_manager->data_update();
		return true;
	}
	else
		return QApplication::event (event);
}

} // namespace Xefis

