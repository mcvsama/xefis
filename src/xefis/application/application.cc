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

	_postponed_update = new QTimer (this);
	_postponed_update->setSingleShot (true);
	_postponed_update->setInterval (1000.f / 30.f);
	QObject::connect (_postponed_update, SIGNAL (timeout()), this, SLOT (data_updated()));

	_offline_updater = new QTimer (this);
	_offline_updater->setInterval (100.f);
	QObject::connect (_offline_updater, SIGNAL (timeout()), this, SLOT (offline_data_updated()));
	_offline_updater->start();

	signal (SIGHUP, s_quit);

	_config_reader->load ("xefis-config.xml");
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
Application::data_updated()
{
	postEvent (this, new DataUpdatedEvent (Timestamp::now()));
	// Restart offline timer:
	_offline_updater->start();
}


void
Application::offline_data_updated()
{
	postEvent (this, new DataUpdatedEvent (Timestamp::now()));
}


void
Application::postponed_data_updated()
{
	if (!_postponed_update->isActive())
		_postponed_update->start();
}


bool
Application::event (QEvent* event)
{
	DataUpdatedEvent* data_update_event = dynamic_cast<DataUpdatedEvent*> (event);

	if (data_update_event)
	{
		_module_manager->data_updated (data_update_event->timestamp());
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

