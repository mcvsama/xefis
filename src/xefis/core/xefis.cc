/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Local:
#include "xefis.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/components/configurator/configurator_widget.h>
#include <xefis/core/executable.h>
#include <xefis/core/machine.h>
#include <xefis/core/machine_manager.h>
#include <xefis/core/system.h>
#include <xefis/core/licenses.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/support/core/single_machine_manager.h>
#include <xefis/support/ui/sound_manager.h>
#include <xefis/xefis_machine.h>

// Neutrino:
#include <neutrino/demangle.h>
#include <neutrino/fail.h>
#include <neutrino/string.h>
#include <neutrino/time_helper.h>
#include <neutrino/work_performer.h>

// System:
#include <signal.h>

// Qt:
#include <QImageReader>

// Standard:
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>


namespace xf {

/**
 * Scaling in Qt6 is horribly broken. You can't get reliably physical DPI of the screen to properly draw stuff.
 * These workarounds are figured out experimentally to disable automatic scaling in Qt6 and make Xefis look more or less not-ugly.
 * Without these literally everything we draw with a QPainter would be blurred/pixelated, with wrong font sizes and the window sizes would be completely
 * bonkers.
 *
 * The size of fonts on KDE/Plasma will still be wrong even with those settings, so I added an option to manually provide font size to be used by Xefis when run
 * in KDE.
 */
bool
fix_broken_qt6_scaling()
{
	auto const overwrite_existing = 1;

	// Without this Qt always reports DPI=96 (what the actual fuck):
	setenv ("QT_ENABLE_HIGHDPI_SCALING", "0", overwrite_existing);

	setenv ("QT_SCALE_FACTOR", "1", overwrite_existing);
	setenv ("QT_USE_PHYSICAL_DPI", "1", overwrite_existing);
	setenv ("QT_SCREEN_SCALE_FACTORS", "", overwrite_existing);
	setenv ("QT_AUTO_SCREEN_SCALE_FACTOR", "0", overwrite_existing);

	return true;
}


// Needs to be executed before main(), so make it a static variable:
static bool fixed = fix_broken_qt6_scaling();


Xefis::Xefis (int& argc, char** argv):
	QApplication (argc, argv)
{
	parse_args (argc, argv);

	// Print warnings about broken rendering when using QT_SCALE_FACTOR:
	if (const char* QT_SCALE_FACTOR = std::getenv ("QT_SCALE_FACTOR"))
	{
		auto const factor = neutrino::parse<double> (QT_SCALE_FACTOR);

		if (std::abs (factor - 1.0) > 1e-4)
		{
			_logger << "Warning: QT_SCALE_FACTOR different than 1 detected.\n"
				<< "Expect stuff to be rendered incorrectly (cropped, wrongly rescaled, etc).\n"
				<< "To have everything working as expected, make sure that the DPI reported by your system\n"
				<< "matches actual DPI of the screen and the QT_SCALE_FACTOR is 1 or unset completely.\n";
		}
	}

	// Print warnings about broken rendering when using QT_FONT_DPI:
	if (std::getenv ("QT_FONT_DPI") != nullptr)
	{
		_logger << "Warning: QT_FONT_DPI detected.\n"
			<< "Xefis works properly if QT_FONT_DPI matches actual physical DPI of the screen.\n"
			<< "Otherwise expect fonts to be too large or too small.\n";
	}

	Exception::log (_logger, [&] {
		QImageReader::setAllocationLimit (512);

		_system = std::make_unique<System> (_logger);
		_graphics = std::make_unique<Graphics> (_logger);
		_machine_manager = make_xefis_machine_manager (*this);

		if (!_machine_manager)
		{
			if (auto machine = make_xefis_machine (*this))
			{
				_machine_manager = std::make_unique<SingleMachineManager> (std::move (machine), *this);
				setup_unix_signals_handler();
			}
			else
				_logger << "Neither machine manager (make_xefis_machine_manager()) nor machine (make_xefis_machine()) was compiled-in." << std::endl;
		}
	});
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


Logger const&
Xefis::fallback_exception_logger()
{
	static LoggerOutput	fallback_exception_logger_output	{ std::cerr };
	static Logger		fallback_exception_logger			{ fallback_exception_logger_output };

	return fallback_exception_logger;
}


void
Xefis::setup_unix_signals_handler()
{
	// Setup a loop to check for UNIX signals:
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
			std::cout << "  --modules-debug-log - dump module settings/sockets information" << std::endl;
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

			_options.modules_debug_log = true;
		}
		else if (arg_name == "--watchdog-write-fd")
		{
			if (arg_value.empty())
				throw MissingValueException (arg_name);

			_options.watchdog_write_fd = neutrino::parse<int> (arg_value);
		}
		else if (arg_name == "--watchdog-read-fd")
		{
			if (arg_value.empty())
				throw MissingValueException (arg_name);

			_options.watchdog_read_fd = neutrino::parse<int> (arg_value);
		}
		else
			throw Exception ("unrecognized option '" + arg_name + "', try --help");
	}
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
	out << "The 'kdtree++' library is distributed under the following license" << endl
		<< "=================================================================" << endl
		<< endl
		<< licenses::lib_kdtreeplusplus << endl
		<< endl;
	out << "The 'type_safe' library is distributed under the following license" << endl
		<< "==================================================================" << endl
		<< endl
		<< licenses::lib_type_safe << endl
		<< endl;
	out << "The 'Microsoft GSL' library is distributed under the following license" << endl
		<< "======================================================================" << endl
		<< endl
		<< licenses::lib_microsoft_gsl << endl
		<< endl;
}

} // namespace xf


/**
 * Default make_xefis_machine() function, used when there's no other provided.
 */
[[gnu::weak]]
std::unique_ptr<xf::Machine>
make_xefis_machine (xf::Xefis&)
{
	return nullptr;
}


/**
 * Default make_xefis_machine_manager() function, used when there's no other provided.
 */
[[gnu::weak]]
std::unique_ptr<xf::MachineManager>
make_xefis_machine_manager (xf::Xefis&)
{
	return nullptr;
}

