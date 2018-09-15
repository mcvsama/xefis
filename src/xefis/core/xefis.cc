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
#include <xefis/core/fail.h>
#include <xefis/core/system.h>
#include <xefis/core/licenses.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/support/system/work_performer.h>
#include <xefis/support/ui/sound_manager.h>
#include <xefis/utility/time_helper.h>
#include <xefis/utility/demangle.h>
#include <xefis/xefis_machine.h>

// Local:
#include "xefis.h"


namespace xf {

Xefis::Xefis (int& argc, char** argv):
	QApplication (argc, argv)
{
	parse_args (argc, argv);

	// Casting QString to std::string|const char* should yield UTF-8 encoded strings.
	// Also encode std::strings and const chars* in UTF-8:
	QTextCodec::setCodecForLocale (QTextCodec::codecForName ("UTF-8"));

	_system = std::make_unique<System> (_logger);
	_graphics = std::make_unique<Graphics> (_logger);
	_machine = ::xefis_machine (this);
	_configurator_widget = std::make_unique<ConfiguratorWidget> (*_machine, nullptr);

	_posix_signals_check_timer = new QTimer (this);
	_posix_signals_check_timer->setSingleShot (false);
	_posix_signals_check_timer->setInterval ((100_ms).in<si::Millisecond>());
	QObject::connect (_posix_signals_check_timer, &QTimer::timeout, [&] {
		if (g_hup_received.load())
		{
			_logger << "HUP received, exiting." << std::endl;
			quit();
		}
	});
	_posix_signals_check_timer->start();
}


bool
Xefis::notify (QObject* receiver, QEvent* event)
{
	try {
		return QApplication::notify (receiver, event);
	}
	catch (...)
	{
		using namespace exception_ops;

		_logger << demangle (typeid (*receiver)) << "/" << demangle (typeid (*event)) << " yielded exception: " << std::endl << std::current_exception() << std::endl;
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

} // namespace xf

