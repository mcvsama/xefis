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

#ifndef XEFIS__APPLICATION__APPLICATION_H__INCLUDED
#define XEFIS__APPLICATION__APPLICATION_H__INCLUDED

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


namespace Xefis {

class NavaidStorage;
class ModuleManager;
class WindowManager;
class SoundManager;
class ConfigReader;
class WorkPerformer;
class ConfiguratorWidget;
class Accounting;
class Airframe;


class Application: public QApplication
{
	Q_OBJECT

  public:
	/**
	 * Thrown when user gives value to a command line option that doesn't take values.
	 */
	class NonValuedArgumentException: public Exception
	{
	  public:
		explicit NonValuedArgumentException (std::string const& argument);
	};

	/**
	 * Thrown when user doesn't give a value to a command line option that takes values.
	 */
	class MissingValueException: public Exception
	{
	  public:
		explicit MissingValueException (std::string const& argument);
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
		// Ctor:
		OptionsHelper (Application*);

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
	Application (int& argc, char** argv);

	// Dtor
	~Application();

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
	ModuleManager*
	module_manager() const;

	/**
	 * Return WindowManager object.
	 */
	WindowManager*
	window_manager() const;

	/**
	 * Return SoundManager object.
	 */
	SoundManager*
	sound_manager() const;

	/**
	 * Return ConfigReader object.
	 */
	ConfigReader*
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
	 * UNIX signal handler. Calls quit() on Application instance.
	 */
	static void
	s_quit (int);

  private:
	static Application*	_application;

	Unique<WorkPerformer>			_work_performer;
	Unique<Accounting>				_accounting;
	Unique<SoundManager>			_sound_manager;
	Unique<NavaidStorage>			_navaid_storage;
	Unique<WindowManager>			_window_manager;
	// Note: it is important that the _module_manager is after _window_manager, so that
	// upon destruction, _module_manager deletes all instruments first, and prevents
	// _window_manager deleting them like they were managed by parent QObjects.
	// Unfortunately Qt doesn't allow inserting a widget into a window without creating parent-child
	// relationship, or at least marking such children not to be deleted by their parent,
	// so we have to make workarounds like this.
	Unique<ModuleManager>			_module_manager;
	Unique<ConfigReader>			_config_reader;
	Unique<ConfiguratorWidget>		_configurator_widget;
	Unique<Airframe>				_airframe;
	Unique<OptionsHelper>			_options_helper;
	QTimer*							_data_updater = nullptr;
	OptionsMap						_options;
};


inline
Application::NonValuedArgumentException::NonValuedArgumentException (std::string const& argument):
	Exception ("argument '" + argument + "' doesn't take any values")
{ }


inline
Application::MissingValueException::MissingValueException (std::string const& argument):
	Exception ("argument '" + argument + "' needs a value")
{ }


inline
Application::OptionsHelper::OptionsHelper (Application* application)
{
	if (application->has_option (Application::Option::WatchdogWriteFd))
		_watchdog_write_fd = boost::lexical_cast<int> (application->option (Application::Option::WatchdogWriteFd));

	if (application->has_option (Application::Option::WatchdogReadFd))
		_watchdog_read_fd = boost::lexical_cast<int> (application->option (Application::Option::WatchdogReadFd));
}


inline Optional<int>
Application::OptionsHelper::watchdog_write_fd() const noexcept
{
	return _watchdog_write_fd;
}


inline Optional<int>
Application::OptionsHelper::watchdog_read_fd() const noexcept
{
	return _watchdog_read_fd;
}


inline Accounting*
Application::accounting() const
{
	return _accounting.get();
}


inline ModuleManager*
Application::module_manager() const
{
	return _module_manager.get();
}


inline WindowManager*
Application::window_manager() const
{
	return _window_manager.get();
}


inline SoundManager*
Application::sound_manager() const
{
	return _sound_manager.get();
}


inline ConfigReader*
Application::config_reader() const
{
	return _config_reader.get();
}


inline NavaidStorage*
Application::navaid_storage() const
{
	return _navaid_storage.get();
}


inline WorkPerformer*
Application::work_performer() const
{
	return _work_performer.get();
}


inline Airframe*
Application::airframe() const
{
	return _airframe.get();
}


inline ConfiguratorWidget*
Application::configurator_widget() const
{
	return _configurator_widget.get();
}


inline bool
Application::has_option (Option option) const
{
	return _options.find (option) != _options.end();
}


inline std::string
Application::option (Option option) const
{
	auto o = _options.find (option);
	if (o != _options.end())
		return o->second;
	else
		return std::string();
}


inline Application::OptionsHelper const&
Application::options() const noexcept
{
	return *_options_helper;
}

} // namespace Xefis

#endif

