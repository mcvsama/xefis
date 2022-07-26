/* vim:ts=4
 *
 * Copyleft 2012…2022  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__APP__XEFIS_H__INCLUDED
#define XEFIS__APP__XEFIS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/configurator/configurator_widget.h>
#include <xefis/core/graphics.h>
#include <xefis/core/system.h>

// Neutrino:
#include <neutrino/logger.h>
#include <neutrino/system.h>

// Qt:
#include <QTimer>
#include <QApplication>

// Lib:
#include <boost/lexical_cast.hpp>

// Standard:
#include <cstddef>
#include <string>
#include <map>


namespace xf {

class Machine;


class Xefis: public QApplication
{
	Q_OBJECT

  public:
	/**
	 * Thrown when user gives value to a command line option that doesn't take values.
	 */
	class NonValuedArgumentException: public Exception
	{
	  public:
		explicit
		NonValuedArgumentException (std::string const& argument);
	};

	/**
	 * Thrown when user doesn't give a value to a command line option that takes values.
	 */
	class MissingValueException: public Exception
	{
	  public:
		explicit
		MissingValueException (std::string const& argument);
	};

	/**
	 * Throw when attempted to access one of the support objects which is not yet initialized.
	 */
	class UninitializedServiceException: public Exception
	{
	  public:
		explicit
		UninitializedServiceException (std::string const& service_name);
	};

	/**
	 * A set of options provided on command-line (or not provided).
	 */
	class Options
	{
	  public:
		std::optional<bool>	modules_debug_log;
		std::optional<int>	watchdog_write_fd;
		std::optional<int>	watchdog_read_fd;
	};

  public:
	// Ctor
	explicit
	Xefis (int& argc, char** argv);

	/**
	 * Override and catch exceptions.
	 */
	bool
	notify (QObject* receiver, QEvent* event) override;

	/**
	 * Tell application to quit main event loop.
	 */
	void
	quit();

	/**
	 * Return System object.
	 */
	[[nodiscard]]
	System&
	system() const;

	/**
	 * Return the Graphics object.
	 */
	Graphics&
	graphics() const;

	/**
	 * Return configurator widget.
	 * May return nullptr, if configurator widget is disabled
	 * (eg. for instrument-less configurations of XEFIS).
	 */
	[[nodiscard]]
	ConfiguratorWidget&
	configurator_widget() const;

	/**
	 * Return Options object that contains values provided on command-line.
	 */
	[[nodiscard]]
	Options const&
	options() const noexcept
		{ return _options; }

	/**
	 * Return logger to use by machines.
	 */
	[[nodiscard]]
	Logger const&
	logger() const noexcept
		{ return _logger; }

	/**
	 * Return global fallback exception logger (one to use when there's no better-fitted one).
	 */
	[[nodiscard]]
	static Logger const&
	fallback_exception_logger();

  private:
	/**
	 * Parse command line options and fill _options map.
	 */
	void
	parse_args (int argc, char** argv);

	/**
	 * Print copyright information on iostream.
	 */
	static void
	print_copyrights (std::ostream&);

  private:
	LoggerOutput						_logger_output	{ std::clog };
	Logger								_logger			{ _logger_output };
	Options								_options;
	QTimer*								_posix_signals_check_timer;

	// Basic subsystems:
	std::unique_ptr<System>				_system;
	std::unique_ptr<ConfiguratorWidget>	_configurator_widget;
	std::unique_ptr<Graphics>			_graphics;
	std::unique_ptr<Machine>			_machine;
};


inline
Xefis::NonValuedArgumentException::NonValuedArgumentException (std::string const& argument):
	Exception ("argument '" + argument + "' doesn't take any values")
{ }


inline
Xefis::MissingValueException::MissingValueException (std::string const& argument):
	Exception ("argument '" + argument + "' needs a value")
{ }


inline
Xefis::UninitializedServiceException::UninitializedServiceException (std::string const& service_name):
	Exception ("service '" + service_name + "' is not initialized")
{ }


inline System&
Xefis::system() const
{
	if (!_system)
		throw UninitializedServiceException ("System");

	return *_system.get();
}


inline Graphics&
Xefis::graphics() const
{
	if (!_graphics)
		throw UninitializedServiceException ("Graphics");

	return *_graphics.get();
}


inline ConfiguratorWidget&
Xefis::configurator_widget() const
{
	if (!_configurator_widget)
		throw UninitializedServiceException ("ConfiguratorWidget");

	return *_configurator_widget.get();
}

} // namespace xf

#endif

