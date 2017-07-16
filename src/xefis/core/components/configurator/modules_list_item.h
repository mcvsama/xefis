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

#ifndef XEFIS__CORE__COMPONENTS__CONFIGURATOR__MODULES_LIST_ITEM_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__CONFIGURATOR__MODULES_LIST_ITEM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QTreeWidgetItem>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module.h>
#include <xefis/core/v1/module_manager.h>


namespace xf {

class ModulesListItem: public QTreeWidgetItem
{
  public:
	// Ctor
	explicit
	ModulesListItem (v1::Module::Pointer const& module_pointer, v1::ModuleManager* module_manager, QTreeWidget* parent);

	/**
	 * Return Module* associated with this item.
	 */
	v1::Module::Pointer const&
	module_pointer() const noexcept;

	/**
	 * Update item (module stats).
	 */
	void
	reload();

  private:
	/**
	 * Sets up widget appereance.
	 */
	void
	setup_appereance();

  private:
	v1::Module::Pointer	_module_pointer;
	v1::ModuleManager*	_module_manager;
};


inline v1::Module::Pointer const&
ModulesListItem::module_pointer() const noexcept
{
	return _module_pointer;
}

} // namespace xf

#endif

