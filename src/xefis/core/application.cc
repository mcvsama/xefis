/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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
#include <xefis/airframe/airframe.h>
#include <xefis/core/services.h>
#include <xefis/core/property_storage.h>
#include <xefis/core/accounting.h>
#include <xefis/core/module_manager.h>
#include <xefis/core/window_manager.h>
#include <xefis/core/sound_manager.h>
#include <xefis/core/config_reader.h>
#include <xefis/core/navaid_storage.h>
#include <xefis/core/work_performer.h>
#include <xefis/components/configurator/configurator_widget.h>

// Local:
#include "application.h"


namespace Xefis {

Application* Application::_application = nullptr;


Application::Application (int& argc, char** argv):
	QApplication (argc, argv)
{
	if (_application)
		throw std::runtime_error ("can create only one Application object");
	_application = this;

	parse_args (argc, argv);

	// Casting QString to std::string|const char* should yield UTF-8 encoded strings.
	// Also encode std::strings and const chars* in UTF-8:
	QTextCodec::setCodecForLocale (QTextCodec::codecForName ("UTF-8"));
	// Init services:
	Services::initialize();
	// Init property storage:
	PropertyStorage::initialize();

	_accounting = std::make_unique<Accounting>();
	_navaid_storage = std::make_unique<NavaidStorage>();
	_module_manager = std::make_unique<ModuleManager> (this);
	_window_manager = std::make_unique<WindowManager> (this);
	_sound_manager = std::make_unique<SoundManager> (this);
	_config_reader = std::make_unique<ConfigReader> (this, _module_manager.get());
	_work_performer = std::make_unique<WorkPerformer> (Services::detected_cores());

	_data_updater = new QTimer (this);
	_data_updater->setInterval (10.f);
	_data_updater->setSingleShot (false);
	QObject::connect (_data_updater, SIGNAL (timeout()), this, SLOT (data_updated()));
	_data_updater->start();

	signal (SIGHUP, s_quit);

	const char* config_file = getenv ("XEFIS_CONFIG");
	if (!config_file)
	{
		std::clog << "XEFIS_CONFIG not set, trying to read default ./xefis-config.xml" << std::endl;
		config_file = "xefis-config.xml";
	}
	_config_reader->load (config_file);

	_airframe = std::make_unique<Airframe> (this, _config_reader->airframe_config());

	_config_reader->process();

	if (_config_reader->has_windows())
		_configurator_widget = std::make_unique<ConfiguratorWidget> (_module_manager.get(), nullptr);
	if (_config_reader->load_navaids())
		_navaid_storage->load();
}


Application::~Application()
{
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
	Time t = Time::now();
	_module_manager->data_updated (t);
	_window_manager->data_updated (t);
}


void
Application::parse_args (int argc, char** argv)
{
	std::vector<std::string> args (argv, argv + argc);
	if (args.empty())
		return;
	args.erase (args.begin(), args.begin() + 1);

	for (auto const& arg: args)
	{
		std::string::size_type eq_pos = arg.find ('=');
		std::string arg_name = arg;
		std::string arg_value;

		if (eq_pos != std::string::npos)
		{
			arg_name = arg.substr (0, eq_pos);
			arg_value = arg.substr (eq_pos + 1);
		}

		if (arg_name == "--help")
		{
			std::cout << "List of available options:" << std::endl;
			std::cout << "  --modules-debug-log - dump module settings/properties information" << std::endl;
			throw QuitInstruction();
		}
		else if (arg_name == "--modules-debug-log")
		{
			if (!arg_value.empty())
				throw NonValuedArgumentException (arg_name);
			_options[Option::ModulesDebugLog] = "";
		}
		else
			throw Exception ("unrecognized option '" + arg_name + "', try --help");
	}
}


void
Application::s_quit (int)
{
	std::clog << "HUP received, exiting." << std::endl;
	if (_application)
		_application->quit();
}

} // namespace Xefis

