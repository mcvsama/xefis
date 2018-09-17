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

#ifndef XEFIS__CORE__XEFIS_H__INCLUDED
#define XEFIS__CORE__XEFIS_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>

// Lib:
#include <boost/lexical_cast.hpp>

// Qt:
#include <QTimer>
#include <QApplication>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/logger.h>
#include <xefis/core/system.h>
#include <xefis/core/components/configurator/configurator_widget.h>


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

	class QuitInstruction { };

  public:
	/**
	 * Contains methods that return command line argument values
	 * with proper type.
	 */
	class OptionsHelper
	{
	  public:
		// Ctor
		explicit
		OptionsHelper (Xefis const&);

		std::optional<int>
		watchdog_write_fd() const noexcept;

		std::optional<int>
		watchdog_read_fd() const noexcept;

	  private:
		std::optional<int>	_watchdog_write_fd;
		std::optional<int>	_watchdog_read_fd;
	};

	// Options related to command line arguments.
	enum class Option
	{
		ModulesDebugLog		= 1UL << 0,
		WatchdogWriteFd		= 1UL << 1,
		WatchdogReadFd		= 1UL << 2,
	};

	typedef std::map<Option, std::string> OptionsMap;

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
	 * Return true if application was run with given command-line option.
	 */
	[[nodiscard]]
	bool
	has_option (Option) const;

	/**
	 * Return value of given command-line options.
	 * If no value was given, return empty string.
	 */
	[[nodiscard]]
	std::string
	option (Option) const;

	/**
	 * Return Options object that contains methods for retrieving
	 * various options with correct type.
	 */
	[[nodiscard]]
	OptionsHelper const&
	options() const noexcept;

	/**
	 * Return logger to use by machines.
	 */
	[[nodiscard]]
	Logger const&
	logger() const noexcept;

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
	OptionsMap							_options;
	QTimer*								_posix_signals_check_timer;

	// Basic subsystems:
	std::unique_ptr<System>				_system;
	std::unique_ptr<ConfiguratorWidget>	_configurator_widget;
	std::unique_ptr<OptionsHelper>		_options_helper;
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


inline
Xefis::OptionsHelper::OptionsHelper (Xefis const& xefis)
{
	// TODO instead of has_option use std::optional<>
	if (xefis.has_option (Xefis::Option::WatchdogWriteFd))
		_watchdog_write_fd = boost::lexical_cast<int> (xefis.option (Xefis::Option::WatchdogWriteFd));

	if (xefis.has_option (Xefis::Option::WatchdogReadFd))
		_watchdog_read_fd = boost::lexical_cast<int> (xefis.option (Xefis::Option::WatchdogReadFd));
}


inline std::optional<int>
Xefis::OptionsHelper::watchdog_write_fd() const noexcept
{
	return _watchdog_write_fd;
}


inline std::optional<int>
Xefis::OptionsHelper::watchdog_read_fd() const noexcept
{
	return _watchdog_read_fd;
}


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


inline bool
Xefis::has_option (Option option) const
{
	return _options.find (option) != _options.end();
}


inline std::string
Xefis::option (Option option) const
{
	if (auto o = _options.find (option); o != _options.end())
		return o->second;
	else
		return std::string();
}


inline Xefis::OptionsHelper const&
Xefis::options() const noexcept
{
	return *_options_helper;
}


inline Logger const&
Xefis::logger() const noexcept
{
	return _logger;
}

} // namespace xf

#endif

