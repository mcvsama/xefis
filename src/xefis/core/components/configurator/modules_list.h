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

#ifndef XEFIS__CORE__COMPONENTS__CONFIGURATOR__MODULES_LIST_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__CONFIGURATOR__MODULES_LIST_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QTimer>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module.h>


namespace xf {

class ModulesList: public QWidget
{
	Q_OBJECT

  public:
	constexpr static int ModuleColumn	= 0;
	constexpr static int StatsAvgColumn	= 1;
	constexpr static int StatsMaxColumn	= 2;

  public:
	// Ctor
	explicit
	ModulesList (ModuleManager* module_manager, QWidget* parent);

	/**
	 * Return ModuleManager.
	 */
	ModuleManager*
	module_manager() const noexcept;

	/**
	 * Deselect any selected module.
	 */
	void
	deselect();

  signals:
	/**
	 * Emitted when user changes module selection.
	 */
	void
	module_selected (Module::Pointer const& module_pointer);

	/**
	 * Emitted when module selection is cleared.
	 */
	void
	none_selected();

  private slots:
	/**
	 * Read list of modules and update items in QTreeWidget.
	 */
	void
	read();

	/**
	 * Called when user changes selection in the list.
	 */
	void
	item_selected (QTreeWidgetItem* current, QTreeWidgetItem* previous);

  private:
	ModuleManager*	_module_manager	= nullptr;
	QTreeWidget*	_list			= nullptr;
	QTimer*			_refresh_timer	= nullptr;
};


inline ModuleManager*
ModulesList::module_manager() const noexcept
{
	return _module_manager;
}

} // namespace xf

#endif

