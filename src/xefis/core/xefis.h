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
#include <QtCore/QTimer>
#include <QtWidgets/QApplication>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/logger.h>


namespace v1 { // XXX
class ModuleManager;
class ConfigReader;
class WindowManager;
}

namespace v2 { // XXX
class Machine;
}

namespace xf {
using namespace v2; // XXX

class NavaidStorage;
class SoundManager;
class WorkPerformer;
class ConfiguratorWidget;
class Accounting;
class Airframe;
class System;


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
		OptionsHelper (Xefis*);

		Optional<int>
		watchdog_write_fd() const noexcept;

		Optional<int>
		watchdog_read_fd() const noexcept;

	  private:
		Optional<int>	_watchdog_write_fd;
		Optional<int>	_watchdog_read_fd;
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

	// Dtor
	~Xefis();

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
	 * Return Accounting object.
	 * Used to track timings and overall response times.
	 */
	Accounting*
	accounting() const;

	/**
	 * Return ModuleManager object.
	 */
	v1::ModuleManager*
	module_manager() const;

	/**
	 * Return WindowManager object.
	 */
	v1::WindowManager*
	window_manager() const;

	/**
	 * Return SoundManager object.
	 */
	SoundManager*
	sound_manager() const;

	/**
	 * Return ConfigReader object.
	 */
	v1::ConfigReader*
	config_reader() const;

	/**
	 * Return pointer to navaid storage.
	 */
	NavaidStorage*
	navaid_storage() const;

	/**
	 * Return WorkPerformer.
	 */
	WorkPerformer*
	work_performer() const;

	/**
	 * Return Airframe object.
	 */
	Airframe*
	airframe() const;

	/**
	 * Return System object.
	 */
	System*
	system() const;

	/**
	 * Return configurator widget.
	 * May return nullptr, if configurator widget is disabled
	 * (eg. for instrument-less configurations of XEFIS).
	 */
	ConfiguratorWidget*
	configurator_widget() const;

	/**
	 * Return true if application was run with given command-line option.
	 */
	bool
	has_option (Option) const;

	/**
	 * Return value of given command-line options.
	 * If no value was given, return empty string.
	 */
	std::string
	option (Option) const;

	/**
	 * Return Options object that contains methods for retrieving
	 * various options with correct type.
	 */
	OptionsHelper const&
	options() const noexcept;

  private slots:
	/**
	 * Called by data updater. Causes call of data_updated() on all modules.
	 */
	void
	data_updated();

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

	/**
	 * UNIX signal handler. Calls quit() on Xefis instance.
	 */
	static void
	s_quit (int);

  private:
	static Xefis*					_xefis;
	static Logger					_logger;

	Unique<System>					_system;
	Unique<WorkPerformer>			_work_performer;
	Unique<Accounting>				_accounting;
	Unique<SoundManager>			_sound_manager;
	Unique<NavaidStorage>			_navaid_storage;
	Unique<v1::WindowManager>		_window_manager;
	// Note: it is important that the _module_manager is after _window_manager, so that
	// upon destruction, _module_manager deletes all instruments first, and prevents
	// _window_manager deleting them like they were managed by parent QObjects.
	// Unfortunately Qt doesn't allow inserting a widget into a window without creating parent-child
	// relationship, or at least marking such children not to be deleted by their parent,
	// so we have to make workarounds like this.
	Unique<v1::ModuleManager>		_module_manager;
	Unique<v1::ConfigReader>		_config_reader;
	Unique<ConfiguratorWidget>		_configurator_widget;
	Unique<Airframe>				_airframe;
	Unique<OptionsHelper>			_options_helper;
	Unique<Machine>					_machine;
	QTimer*							_data_updater = nullptr;
	OptionsMap						_options;
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
Xefis::OptionsHelper::OptionsHelper (Xefis* xefis)
{
	if (xefis->has_option (Xefis::Option::WatchdogWriteFd))
		_watchdog_write_fd = boost::lexical_cast<int> (xefis->option (Xefis::Option::WatchdogWriteFd));

	if (xefis->has_option (Xefis::Option::WatchdogReadFd))
		_watchdog_read_fd = boost::lexical_cast<int> (xefis->option (Xefis::Option::WatchdogReadFd));
}


inline Optional<int>
Xefis::OptionsHelper::watchdog_write_fd() const noexcept
{
	return _watchdog_write_fd;
}


inline Optional<int>
Xefis::OptionsHelper::watchdog_read_fd() const noexcept
{
	return _watchdog_read_fd;
}


inline Accounting*
Xefis::accounting() const
{
	if (!_accounting)
		throw UninitializedServiceException ("Accounting");
	return _accounting.get();
}


inline v1::ModuleManager*
Xefis::module_manager() const
{
	if (!_module_manager)
		throw UninitializedServiceException ("ModuleManager");
	return _module_manager.get();
}


inline v1::WindowManager*
Xefis::window_manager() const
{
	if (!_window_manager)
		throw UninitializedServiceException ("WindowManager");
	return _window_manager.get();
}


inline SoundManager*
Xefis::sound_manager() const
{
	if (!_sound_manager)
		throw UninitializedServiceException ("SoundManager");
	return _sound_manager.get();
}


inline v1::ConfigReader*
Xefis::config_reader() const
{
	if (!_config_reader)
		throw UninitializedServiceException ("ConfigReader");
	return _config_reader.get();
}


inline NavaidStorage*
Xefis::navaid_storage() const
{
	if (!_navaid_storage)
		throw UninitializedServiceException ("NavaidStorage");
	return _navaid_storage.get();
}


inline WorkPerformer*
Xefis::work_performer() const
{
	if (!_work_performer)
		throw UninitializedServiceException ("WorkPerformer");
	return _work_performer.get();
}


inline Airframe*
Xefis::airframe() const
{
	if (!_airframe)
		throw UninitializedServiceException ("Airframe");
	return _airframe.get();
}


inline System*
Xefis::system() const
{
	if (!_system)
		throw UninitializedServiceException ("System");
	return _system.get();
}


inline ConfiguratorWidget*
Xefis::configurator_widget() const
{
	if (!_configurator_widget)
		throw UninitializedServiceException ("ConfiguratorWidget");
	return _configurator_widget.get();
}


inline bool
Xefis::has_option (Option option) const
{
	return _options.find (option) != _options.end();
}


inline std::string
Xefis::option (Option option) const
{
	auto o = _options.find (option);
	if (o != _options.end())
		return o->second;
	else
		return std::string();
}


inline Xefis::OptionsHelper const&
Xefis::options() const noexcept
{
	return *_options_helper;
}

} // namespace xf

#endif

