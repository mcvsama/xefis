/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__COMPONENTS__MODULE_MANAGER_H__INCLUDED
#define XEFIS__COMPONENTS__MODULE_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Qt:
#include <QtCore/QString>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

class Module;

class ModuleManager
{
	typedef std::set<Module*> Modules;

  public:
	virtual ~ModuleManager();

	/**
	 * Add module by name.
	 * If parent is nullptr, widget will not be shown.
	 */
	Module*
	load_module (QString const& name, QWidget* parent);

  private:
	Modules	_modules;
};


class ModuleNotFoundException: public Exception
{
  public:
	ModuleNotFoundException (std::string const& message):
		Exception (message)
	{ }
};

} // namespace Xefis

#endif

