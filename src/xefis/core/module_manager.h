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

#ifndef XEFIS__CORE__MODULE_MANAGER_H__INCLUDED
#define XEFIS__CORE__MODULE_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Qt:
#include <QtCore/QString>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

class Module;
class Application;

class ModuleManager
{
	typedef std::set<Module*> Modules;

  public:
	// Ctor
	ModuleManager (Application*);

	// Dtor
	virtual ~ModuleManager();

	/**
	 * Access Application object.
	 */
	Application*
	application() const;

	/**
	 * Add module by name.
	 * If parent is nullptr, widget will not be shown.
	 */
	Module*
	load_module (QString const& name, QDomElement const& config, QWidget* parent);

	/**
	 * Signal that the data in property tree has been updated.
	 * Forward call to all loaded modules.
	 */
	void
	data_update();

  private:
	Application*	_application = nullptr;
	Modules			_modules;
};


class ModuleNotFoundException: public Exception
{
  public:
	ModuleNotFoundException (std::string const& message):
		Exception (message)
	{ }
};


inline Application*
ModuleManager::application() const
{
	return _application;
}

} // namespace Xefis

#endif

