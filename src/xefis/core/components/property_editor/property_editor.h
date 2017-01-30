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

#ifndef XEFIS__CORE__COMPONENTS__PROPERTY_EDITOR__PROPERTY_EDITOR_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__PROPERTY_EDITOR__PROPERTY_EDITOR_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QTimer>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property_tree_widget.h"


namespace xf {

class PropertyEditor: public QWidget
{
	Q_OBJECT

	class EditorWidget
	{
	  public:
		EditorWidget (QWidget* parent);
	};

  public:
	PropertyEditor (PropertyNode* root_node, QWidget* parent);

  public slots:
	/**
	 * Display context menu for a property.
	 */
	void
	handle_context_menu_request (QTreeWidgetItem*, QPoint const& pos);

  private:
	void
	set_line_edit_color (QColor);

  private slots:
	void
	item_selected();

	void
	item_changed (QTreeWidgetItem*, int column);

	void
	update_item();

	void
	reset_item();

	void
	reset_error();

	void
	focus_editor (QTreeWidgetItem*, int column);

	void
	context_item_set_nil();

  private:
	QColor					_accepted_color			= { 0x60, 0xff, 0x70 };
	QColor					_error_color			= { 0xff, 0xa7, 0xa7 };
	QColor					_normal_color			= Qt::white;
	PropertyTreeWidget*		_property_tree_widget;
	PropertyTreeWidgetItem*	_context_item			= nullptr;
	QLineEdit*				_editable_value;
	QPushButton*			_update_button;
	QPushButton*			_set_nil_button;
	QTimer*					_accepted_blink_timer;
};

} // namespace xf

#endif

