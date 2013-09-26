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

#ifndef XEFIS__APPLICATION__APPLICATION_H__INCLUDED
#define XEFIS__APPLICATION__APPLICATION_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QTimer>
#include <QtWidgets/QApplication>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

class NavaidStorage;
class ModuleManager;
class ConfigReader;
class WorkPerformer;
class ConfiguratorWidget;
class Accounting;


class Application: public QApplication
{
	Q_OBJECT

	class DataUpdatedEvent: public QEvent
	{
	  public:
		DataUpdatedEvent (Time);

		Time
		time() const;

	  private:
		Time _time;
	};

  public:
	// Ctor
	Application (int argc, char** argv);

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
	 * Return configurator widget.
	 * May return nullptr, if configurator widget is disabled
	 * (eg. for instrument-less configurations of XEFIS).
	 */
	ConfiguratorWidget*
	configurator_widget() const;

  public slots:
	/**
	 * Indicate that the data in property tree has been updated
	 * from an IO module.
	 */
	void
	data_updated();

	/**
	 * Called by offline data updater. Similar to data updated,
	 * but will not restart offline timer.
	 */
	void
	offline_data_updated();

	/**
	 * Indicate that the data was updated, but the update signal
	 * can be send later.
	 */
	void
	postponed_data_updated();

  protected:
	bool
	event (QEvent*) override;

  private:
	/**
	 * UNIX signal handler. Calls quit() on Application instance.
	 */
	static void
	s_quit (int);

  private:
	static Application*	_application;

	Accounting*			_accounting				= nullptr;
	NavaidStorage*		_navaid_storage			= nullptr;
	ModuleManager*		_module_manager			= nullptr;
	ConfigReader*		_config_reader			= nullptr;
	ConfiguratorWidget*	_configurator_widget	= nullptr;
	QTimer*				_postponed_update		= nullptr;
	QTimer*				_offline_updater		= nullptr;
	WorkPerformer*		_work_performer			= nullptr;
};


inline
Application::DataUpdatedEvent::DataUpdatedEvent (Time time):
	QEvent (QEvent::User),
	_time (time)
{ }


inline Time
Application::DataUpdatedEvent::time() const
{
	return _time;
}


inline Accounting*
Application::accounting() const
{
	return _accounting;
}


inline ModuleManager*
Application::module_manager() const
{
	return _module_manager;
}


inline NavaidStorage*
Application::navaid_storage() const
{
	return _navaid_storage;
}


inline WorkPerformer*
Application::work_performer() const
{
	return _work_performer;
}


inline ConfiguratorWidget*
Application::configurator_widget() const
{
	return _configurator_widget;
}

} // namespace Xefis

#endif

