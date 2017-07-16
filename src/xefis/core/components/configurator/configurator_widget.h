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

#ifndef XEFIS__CORE__COMPONENTS__CONFIGURATOR__CONFIGURATOR_WIDGET_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__CONFIGURATOR__CONFIGURATOR_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QLabel>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/property_editor/property_editor.h>
#include <xefis/core/components/data_recorder/data_recorder.h>
#include <xefis/core/v1/window.h>

// Local:
#include "modules_list.h"


namespace xf {

class ConfiguratorWidget: public QWidget
{
	Q_OBJECT

	/**
	 * OwnershipBreakingDecorator widget that ensures its child widget is NOT deleted
	 * when decorator is deleted. Used to break Qt's parent-child relationship
	 * when it comes to pointer ownership (since Qt doesn't have its own
	 * mechanism for this).
	 *
	 * Also - lays out the child widget.
	 */
	class OwnershipBreakingDecorator: public QWidget
	{
	  public:
		// Ctor
		explicit
		OwnershipBreakingDecorator (QWidget* child, QWidget* parent);

		// Dtor
		~OwnershipBreakingDecorator();

	  private:
		QWidget* _child;
	};

	/**
	 * Configuration widget for module.
	 * Contains generic config widgets, module's configurator widget,
	 * and other stuff.
	 */
	class GeneralModuleWidget: public QWidget
	{
	  public:
		// Ctor
		explicit
		GeneralModuleWidget (Xefis*, v1::Module*, ConfiguratorWidget*, QWidget* parent);

		/**
		 * Return module.
		 */
		v1::Module*
		module() const noexcept;

		/**
		 * Reload module.
		 */
		void
		reload_module();

	  private:
		Xefis*				_xefis;
		v1::Module*			_module;
		v1::Module::Pointer	_module_ptr;
		ConfiguratorWidget*	_configurator_widget;
	};

  public:
	// Ctor
	explicit
	ConfiguratorWidget (Xefis*, QWidget* parent);

	v1::Window*
	owning_window() const;

	void
	set_owning_window (v1::Window*);

  private:
	void
	module_selected (v1::Module::Pointer const&);

	void
	none_selected();

	/**
	 * Causes module widget to be reloaded.
	 * The one passed in parameter will be deleted.
	 */
	void
	reload_module_widget (GeneralModuleWidget*);

  private:
	Xefis*				_xefis					= nullptr;
	PropertyEditor*		_property_editor		= nullptr;
	ModulesList*		_modules_list			= nullptr;
	QStackedWidget*		_modules_stack			= nullptr;
	DataRecorder*		_data_recorder			= nullptr;
	QTabWidget*			_tabs					= nullptr;
	v1::Window*			_owning_window			= nullptr;
	QLabel*				_no_module_selected		= nullptr;
	std::map<v1::Module*, Shared<GeneralModuleWidget>>
						_general_module_widgets;
};


inline v1::Module*
ConfiguratorWidget::GeneralModuleWidget::module() const noexcept
{
	return _module;
}


inline v1::Window*
ConfiguratorWidget::owning_window() const
{
	return _owning_window;
}


inline void
ConfiguratorWidget::set_owning_window (v1::Window* window)
{
	_owning_window = window;
}

} // namespace xf

#endif

