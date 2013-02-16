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
#include <xefis/utility/timestamp.h>


namespace Xefis {

class NavaidStorage;
class ModuleManager;
class ConfigReader;


class Application: public QApplication
{
	Q_OBJECT

	class DataUpdatedEvent: public QEvent
	{
	  public:
		DataUpdatedEvent (Timestamp);

		Timestamp
		timestamp() const;

	  private:
		Timestamp _timestamp;
	};

  public:
	// Ctor
	Application (int argc, char** argv);

	// Dtor
	~Application();

	/**
	 * Tell application to quit main event loop.
	 */
	void
	quit();

	/**
	 * Return pointer to navaid storage.
	 */
	NavaidStorage*
	navaid_storage() const;

  public slots:
	/**
	 * Indicate that the data in property tree has been updated
	 * from an IO module.
	 */
	void
	data_updated();

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

	NavaidStorage*		_navaid_storage	= nullptr;
	ModuleManager*		_module_manager	= nullptr;
	ConfigReader*		_config_reader	= nullptr;
	QTimer*				_postponed_update;
	QTimer*				_offline_updater;
};


inline
Application::DataUpdatedEvent::DataUpdatedEvent (Timestamp timestamp):
	QEvent (QEvent::User),
	_timestamp (timestamp)
{ }


inline Timestamp
Application::DataUpdatedEvent::timestamp() const
{
	return _timestamp;
}


inline NavaidStorage*
Application::navaid_storage() const
{
	return _navaid_storage;
}

} // namespace Xefis

#endif

