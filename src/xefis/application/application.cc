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
#include <cstdlib>
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
#include <xefis/components/configurator/configurator_widget.h>
#include <xefis/core/property_storage.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/config_reader.h>
#include <xefis/core/navaid_storage.h>
#include <xefis/core/work_performer.h>

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
	_work_performer = new WorkPerformer (Services::detected_cores());

	_postponed_update = new QTimer (this);
	_postponed_update->setSingleShot (true);
	_postponed_update->setInterval (1000.f / 30.f);
	QObject::connect (_postponed_update, SIGNAL (timeout()), this, SLOT (data_updated()));

	_offline_updater = new QTimer (this);
	_offline_updater->setInterval (100.f);
	QObject::connect (_offline_updater, SIGNAL (timeout()), this, SLOT (offline_data_updated()));
	_offline_updater->start();

	signal (SIGHUP, s_quit);

	const char* config_file = getenv ("XEFIS_CONFIG");
	if (!config_file)
	{
		std::clog << "XEFIS_CONFIG not set, trying to read default ./xefis-config.xml" << std::endl;
		config_file = "xefis-config.xml";
	}
	_config_reader->load (config_file);

	if (_config_reader->has_windows())
		_configurator_widget = new ConfiguratorWidget (nullptr);
	if (_config_reader->load_navaids())
		_navaid_storage->load();
}


Application::~Application()
{
	delete _work_performer;
	delete _configurator_widget;
	delete _config_reader;
	delete _module_manager;
	delete _navaid_storage;
	Services::deinitialize();
	_application = nullptr;
}


bool
Application::notify (QObject* receiver, QEvent* event)
{
	try {
		return QApplication::notify (receiver, event);
	}
	catch (Exception const& e)
	{
		std::clog << typeid (*receiver).name() << "/" << typeid (*event).name() << " yielded Xefis::Exception:" << std::endl << e << std::endl;
	}
	catch (boost::exception const& e)
	{
		std::clog << typeid (*receiver).name() << "/" << typeid (*event).name() << " yielded boost::exception " << typeid (e).name() << std::endl;
	}
	catch (std::exception const& e)
	{
		std::clog << typeid (*receiver).name() << "/" << typeid (*event).name() << " yielded std::exception " << typeid (e).name() << std::endl;
	}
	catch (...)
	{
		std::clog << typeid (*receiver).name() << "/" << typeid (*event).name() << " yielded unknown exception" << std::endl;
	}

	return false;
}


void
Application::quit()
{
	closeAllWindows();
	QApplication::quit();
}


void
Application::data_updated()
{
	postEvent (this, new DataUpdatedEvent (Time::now()));
	// Restart offline timer:
	_offline_updater->start();
}


void
Application::offline_data_updated()
{
	postEvent (this, new DataUpdatedEvent (Time::now()));
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
		_module_manager->data_updated (data_update_event->time());
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

