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

// Standard:
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>

// System:
#include <signal.h>

// Qt:
#include <QtCore/QTextCodec>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/configurator/configurator_widget.h>
#include <xefis/core/services.h>
#include <xefis/core/v1/property_storage.h>
#include <xefis/core/accounting.h>
#include <xefis/core/v1/module_manager.h>
#include <xefis/core/v1/window_manager.h>
#include <xefis/core/v1/config_reader.h>
#include <xefis/core/system.h>
#include <xefis/core/licenses.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/support/navigation/navaid_storage.h>
#include <xefis/support/system/work_performer.h>
#include <xefis/support/ui/sound_manager.h>
#include <xefis/utility/time_helper.h>
#include <xefis/utility/demangle.h>
// TODO machine
#include <configs/cthulhu_gcs/cthulhu_gcs.h>

// Local:
#include "xefis.h"


namespace xf {

Xefis*	Xefis::_xefis = nullptr;
Logger	Xefis::_logger;


Xefis::Xefis (int& argc, char** argv):
	QApplication (argc, argv)
{
	if (_xefis)
		throw std::runtime_error ("can create only one Xefis object");
	_xefis = this;
	_logger.set_prefix ("<xefis>");

	parse_args (argc, argv);

	// Casting QString to std::string|const char* should yield UTF-8 encoded strings.
	// Also encode std::strings and const chars* in UTF-8:
	QTextCodec::setCodecForLocale (QTextCodec::codecForName ("UTF-8"));
	// Init services:
	Services::initialize();
	// Init property storage:
	v1::PropertyStorage::initialize();

	_system = std::make_unique<System>();
	_work_performer = std::make_unique<WorkPerformer> (std::thread::hardware_concurrency());
	_accounting = std::make_unique<Accounting>();
	_sound_manager = std::make_unique<SoundManager>();
	_navaid_storage = std::make_unique<NavaidStorage>();
	_window_manager = std::make_unique<v1::WindowManager>();
	_module_manager = std::make_unique<v1::ModuleManager> (this);
	_config_reader = std::make_unique<v1::ConfigReader> (this, _module_manager.get());

	signal (SIGHUP, s_quit);

	const char* config_file = getenv ("XEFIS_CONFIG");
	if (!config_file)
	{
		_logger << "XEFIS_CONFIG not set, trying to read default ./xefis-config.xml" << std::endl;
		config_file = "xefis-config.xml";
	}
	_config_reader->load (config_file);

	_airframe = std::make_unique<Airframe> (this, _config_reader->airframe_config());

	_config_reader->process_settings();

	if (_config_reader->load_navaids())
		_navaid_storage->load();

	_config_reader->process_modules();
	_config_reader->process_windows();

	if (_config_reader->has_windows())
		_configurator_widget = std::make_unique<ConfiguratorWidget> (this, nullptr);

	_data_updater = new QTimer (this);
	_data_updater->setInterval ((1.0 / _config_reader->update_frequency()).quantity<Millisecond>());
	_data_updater->setSingleShot (false);
	QObject::connect (_data_updater, SIGNAL (timeout()), this, SLOT (data_updated()));
	_data_updater->start();

	// TODO this is a hack, make it configurable or runtime selectable
	_machine = std::make_unique<CthulhuGCS> (this);
}


Xefis::~Xefis()
{
	Services::deinitialize();
	_xefis = nullptr;
}


bool
Xefis::notify (QObject* receiver, QEvent* event)
{
	try {
		return QApplication::notify (receiver, event);
	}
	catch (Exception const& e)
	{
		_logger << demangle (typeid (*receiver)) << "/" << demangle (typeid (*event)) << " yielded xf::Exception:" << std::endl << e << std::endl;
	}
	catch (boost::exception const& e)
	{
		_logger << demangle (typeid (*receiver)) << "/" << demangle (typeid (*event)) << " yielded boost::exception " << demangle (typeid (e)) << std::endl;
	}
	catch (std::exception const& e)
	{
		_logger << demangle (typeid (*receiver)) << "/" << demangle (typeid (*event)) << " yielded std::exception " << demangle (typeid (e)) << std::endl;
	}
	catch (...)
	{
		_logger << demangle (typeid (*receiver)) << "/" << demangle (typeid (*event)) << " yielded unknown exception" << std::endl;
	}

	return false;
}


void
Xefis::quit()
{
	closeAllWindows();
	QApplication::quit();
}


void
Xefis::data_updated()
{
	Time t = TimeHelper::now();
	_module_manager->data_updated (t);
	_window_manager->data_updated (t);
}


void
Xefis::parse_args (int argc, char** argv)
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
			std::cout << "  --copyright         - print license info" << std::endl;
			throw QuitInstruction();
		}
		else if (arg_name == "--copyright")
		{
			print_copyrights (std::cout);
			throw QuitInstruction();
		}
		else if (arg_name == "--modules-debug-log")
		{
			if (!arg_value.empty())
				throw NonValuedArgumentException (arg_name);
			_options[Option::ModulesDebugLog] = "";
		}
		else if (arg_name == "--watchdog-write-fd")
		{
			if (arg_value.empty())
				throw MissingValueException (arg_name);
			_options[Option::WatchdogWriteFd] = arg_value;
		}
		else if (arg_name == "--watchdog-read-fd")
		{
			if (arg_value.empty())
				throw MissingValueException (arg_name);
			_options[Option::WatchdogReadFd] = arg_value;
		}
		else
			throw Exception ("unrecognized option '" + arg_name + "', try --help");
	}

	_options_helper = std::make_unique<OptionsHelper> (this);
}


void
Xefis::print_copyrights (std::ostream& out)
{
	using std::endl;

	out	<< "Main program license" << std::endl
		<< "====================" << endl
		<< endl
		<< licenses::main << endl
		<< endl;
	out << "Fonts" << endl
		<< "=====" << endl
		<< licenses::font_crystal << endl
		<< endl;
	out << "The 'half' library is distributed under the following license" << endl
		<< "=============================================================" << endl
		<< endl
		<< licenses::lib_half << endl
		<< endl;
	out << "The 'kdtree++' library is distributed under the following license" << endl
		<< "=================================================================" << endl
		<< endl
		<< licenses::lib_kdtreeplusplus << endl
		<< endl;
}


void
Xefis::s_quit (int)
{
	_logger << "HUP received, exiting." << std::endl;
	if (_xefis)
		_xefis->quit();
}

} // namespace xf

