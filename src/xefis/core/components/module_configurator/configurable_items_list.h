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

#ifndef XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__CONFIGURABLE_ITEMS_LIST_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__CONFIGURABLE_ITEMS_LIST_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/core/module.h>
#include <xefis/core/processing_loop.h>
#include <xefis/core/screen.h>

// Qt:
#include <QtCore/QTimer>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeWidget>

// Standard:
#include <cstddef>


// TODO Fix this to xf::configurator when Qt MOC parsing bugs get fixed
namespace xf {
namespace configurator {

class ConfigurableItemsList: public QWidget
{
	Q_OBJECT

  public:
	constexpr static int NameColumn	= 0;

  public:
	// Ctor
	explicit
	ConfigurableItemsList (Machine&, QWidget* parent);

	/**
	 * Deselect any selected module.
	 */
	void
	deselect();

  signals:
	/**
	 * Emitted when user selects a ProcessingLoop item.
	 */
	void
	processing_loop_selected (ProcessingLoop&);

	/**
	 * Emitted when user selects a Module (also Instrument) item.
	 */
	void
	module_selected (Module&);

	/**
	 * Emitted when user selects a Screen item.
	 */
	void
	screen_selected (Screen&);

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
	template<class TempContainer, class ItemToPointerMapper>
		static void
		populate_subtree (QTreeWidgetItem& tree, TempContainer& container, ItemToPointerMapper&& item_to_pointer);

  private:
	Machine&						_machine;
	QTreeWidget*					_list			= nullptr;
	QTimer*							_refresh_timer	= nullptr;
	std::vector<ProcessingLoop*>	_tmp_processing_loop_ptrs;
	std::vector<Screen*>			_tmp_screen_ptrs;
	std::vector<Module*>			_tmp_module_ptrs;
};

} // namespce configurator
} // namespace xf

#endif

