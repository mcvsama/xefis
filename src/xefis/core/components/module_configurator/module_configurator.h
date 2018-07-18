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

#ifndef XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__MODULE_CONFIGURATOR_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__MODULE_CONFIGURATOR_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QLabel>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>

// Local:
#include "module_widget.h"
#include "modules_list.h"


namespace xf {

class ModuleConfigurator: public QWidget
{
  public:
	// Ctor
	explicit
	ModuleConfigurator (Machine&, QWidget* parent);

  private:
	void
	module_selected (BasicModule&);

	void
	none_selected();

	/**
	 * Causes module widget to be reloaded.
	 * The one passed in parameter will be deleted.
	 */
	void
	reload_module_widget (ModuleWidget*);

  private:
	Machine&		_machine;
	ModulesList*	_modules_list			= nullptr;
	QStackedWidget*	_modules_stack			= nullptr;
	QLabel*			_no_module_selected		= nullptr;
	std::map<BasicModule*, std::shared_ptr<ModuleWidget>>
					_module_widgets;
};

} // namespace xf

#endif

