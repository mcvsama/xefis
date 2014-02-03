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
	class NonValuedArgumentException: public Exception
	{
	  public:
		explicit NonValuedArgumentException (std::string const& argument);
	};

	class QuitInstruction { };

  public:
	// Options related to command line arguments.
	enum class Option
	{
		ModulesDebugLog		= 1UL << 0,
		WatchDogFD			= 1UL << 1,
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
	 * UNIX signal handler. Calls quit() on Application instance.
	 */
	static void
	s_quit (int);

  private:
	static Application*	_application;

	Unique<Accounting>				_accounting;
	Unique<NavaidStorage>			_navaid_storage;
	Unique<WindowManager>			_window_manager;
	// Note: it is important that the _module_manager is after _window_manager, so that
	// upon destruction, _module_manager deletes all instruments first, and prevents
	// _window_manager deleting them like they were managed by parent QObjects.
	// Unfortunately Qt doesn't allow inserting a widget into a window without creating parent-child
	// relationship, so we have to make workarounds like this.
	Unique<ModuleManager>			_module_manager;
	Unique<SoundManager>			_sound_manager;
	Unique<ConfigReader>			_config_reader;
	Unique<ConfiguratorWidget>		_configurator_widget;
	Unique<WorkPerformer>			_work_performer;
	Unique<Airframe>				_airframe;
	QTimer*							_data_updater = nullptr;
	OptionsMap						_options;
};


inline
Application::NonValuedArgumentException::NonValuedArgumentException (std::string const& argument):
	Exception ("argument '" + argument + "' doesn't take any values")
{ }


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

} // namespace Xefis

#endif

