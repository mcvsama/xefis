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

#ifndef XEFIS__COMPONENTS__CONFIGURATOR__CONFIGURATOR_WIDGET_H__INCLUDED
#define XEFIS__COMPONENTS__CONFIGURATOR__CONFIGURATOR_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QLabel>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/components/property_editor/property_editor.h>
#include <xefis/core/window.h>

// Local:
#include "modules_list.h"


namespace Xefis {

class ConfiguratorWidget: public QWidget
{
	Q_OBJECT

  public:
	// Ctor
	ConfiguratorWidget (ModuleManager* module_manager, QWidget* parent);

	Window*
	owning_window() const;

	void
	set_owning_window (Window*);

  private slots:
	void
	module_selected (Module::Pointer const&);

  private:
	/**
	 * Create a decorator for given widget.
	 */
	void
	decorate_widget (QWidget* configurator_widget);

  private:
	ModuleManager*					_module_manager			= nullptr;
	PropertyEditor*					_property_editor		= nullptr;
	ModulesList*					_modules_list			= nullptr;
	QStackedWidget*					_modules_stack			= nullptr;
	QTabWidget*						_tabs					= nullptr;
	Window*							_owning_window			= nullptr;
	QLabel*							_no_config_placeholder	= nullptr;
	std::map<QWidget*, QWidget*>	_config_decorators;
};


inline Window*
ConfiguratorWidget::owning_window() const
{
	return _owning_window;
}


inline void
ConfiguratorWidget::set_owning_window (Window* window)
{
	_owning_window = window;
}

} // namespace Xefis

#endif

