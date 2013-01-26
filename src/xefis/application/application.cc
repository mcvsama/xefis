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

// System:
#include <signal.h>

// Qt:
#include <QtCore/QTextCodec>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/services.h>
#include <xefis/core/property_storage.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/config_reader.h>
#include <xefis/core/navaid_storage.h>

// Local:
#include "application.h"


namespace Xefis {

Application* Application::_application = nullptr;


Application::Application (int argc, char** argv):
	QApplication (argc, argv)
{
	if (_application)
		throw std::runtime_error ("can create only one Application object");
	_application = this;

	// Casting QString to std::string|const char* should yield UTF-8 encoded strings.
	// Also encode std::strings and const chars* in UTF-8:
	QTextCodec::setCodecForLocale (QTextCodec::codecForName ("UTF-8"));
	// Init services:
	Services::initialize();
	// Init property storage:
	PropertyStorage::initialize();

	_navaid_storage = new NavaidStorage();
	_module_manager = new ModuleManager (this);
	_config_reader = new ConfigReader (this, _module_manager);
	_config_reader->load ("xefis-config.xml");

	signal (SIGHUP, s_quit);
}


Application::~Application()
{
	delete _config_reader;
	delete _module_manager;
	delete _navaid_storage;
	Services::deinitialize();
	_application = nullptr;
}


void
Application::quit()
{
	closeAllWindows();
}


void
Application::data_update()
{
	postEvent (this, new DataUpdateEvent (Timestamp::now()));
}


bool
Application::event (QEvent* event)
{
	DataUpdateEvent* data_update_event = dynamic_cast<DataUpdateEvent*> (event);

	if (data_update_event)
	{
		_module_manager->data_update (data_update_event->timestamp());
		return true;
	}
	else
		return QApplication::event (event);
}


void
Application::s_quit (int)
{
	std::clog << "HUP received, exiting." << std::endl;
	if (_application)
		_application->quit();
}

} // namespace Xefis

