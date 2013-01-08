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
#include <QtWidgets/QApplication>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

class ModuleManager;
class ConfigReader;


class Application: public QApplication
{
	class DataUpdateEvent: public QEvent
	{
	  public:
		DataUpdateEvent();
	};

  public:
	static Application*
	create (int argc, char** argv);

	static void
	destroy();

	static Application*
	get();

  private:
	// Ctor:
	Application (int argc, char** argv);

  public:
	// Dtor:
	~Application();

	/**
	 * Tell application to quit main event loop.
	 */
	void
	quit();

	/**
	 * Indicate that the data in property tree has been updated
	 * from an IO module.
	 */
	void
	data_update();

  protected:
	bool
	event (QEvent*) override;

  private:
	static Application*	_application;
	ModuleManager*		_module_manager	= nullptr;
	ConfigReader*		_config_reader	= nullptr;
};


inline
Application::DataUpdateEvent::DataUpdateEvent():
	QEvent (QEvent::User)
{ }


inline Application*
Application::create (int argc, char** argv)
{
	return _application = new Application (argc, argv);
}


inline void
Application::destroy()
{
	delete _application;
	_application = nullptr;
}


inline Application*
Application::get()
{
	return _application;
}


/*
 * Global functions
 */


inline Application*
xefis()
{
	return Application::get();
}

} // namespace Xefis

#endif

