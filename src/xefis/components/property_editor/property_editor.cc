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

// Standard:
#include <cstddef>
#include <stdexcept>
#include <set>

// Qt:
#include <QtWidgets/QLayout>
#include <QtWidgets/QMessageBox>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property_editor.h"
#include "property_tree_widget_item.h"


namespace Xefis {

PropertyEditor::PropertyEditor (PropertyNode* root_node, QWidget* parent):
	QWidget (parent)
{
	_property_tree_widget = new PropertyTreeWidget (root_node, this);
	QObject::connect (_property_tree_widget, SIGNAL (itemSelectionChanged()), this, SLOT (item_selected()));
	QObject::connect (_property_tree_widget, SIGNAL (itemClicked (QTreeWidgetItem*, int)), this, SLOT (item_changed (QTreeWidgetItem*, int)));

	_editable_value = new QLineEdit (this);
	_normal_color = _editable_value->palette().color (QPalette::Base);
	QObject::connect (_editable_value, SIGNAL (textChanged (const QString&)), this, SLOT (reset_error()));
	QObject::connect (_editable_value, SIGNAL (returnPressed()), this, SLOT (update_item()));
	_editable_value->adjustSize();

	_update_button = new QPushButton ("Update", this);
	_update_button->setDefault (true);
	_update_button->setFixedHeight (_editable_value->height());;
	QObject::connect (_update_button, SIGNAL (clicked()), this, SLOT (update_item()));

	QLayout* value_layout = new QHBoxLayout();
	value_layout->setMargin (0);
	value_layout->setSpacing (WidgetSpacing);
	value_layout->addWidget (_editable_value);
	value_layout->addWidget (_update_button);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (WidgetSpacing);
	layout->addLayout (value_layout);
	layout->addWidget (_property_tree_widget);

	_accepted_blink_timer = new QTimer (this);
	_accepted_blink_timer->setInterval (300);
	_accepted_blink_timer->setSingleShot (true);
	QObject::connect (_accepted_blink_timer, SIGNAL (timeout()), this, SLOT (reset_error()));

	item_selected();
}


void
PropertyEditor::set_line_edit_color (QColor color)
{
	QPalette palette = _editable_value->palette();
	palette.setColor (QPalette::Base, color);
	_editable_value->setPalette (palette);
}


void
PropertyEditor::item_selected()
{
	PropertyNode* node = _property_tree_widget->selected_property_node();
	if (node)
	{
		QWidget* widgets[] = { _editable_value, _update_button };

		TypedPropertyValueNode* val_node = dynamic_cast<TypedPropertyValueNode*> (node);
		if (!val_node)
		{
			_editable_value->setText ("");
			for (QWidget* w: widgets)
				w->setEnabled (false);
		}
		else
		{
			_editable_value->setText (val_node->stringify().c_str());
			for (QWidget* w: widgets)
				w->setEnabled (true);
		}
	}
}


void
PropertyEditor::item_changed (QTreeWidgetItem* item, int column)
{
	if (column != PropertyTreeWidget::ValueColumn)
		return;

	PropertyTreeWidgetItem* prop_item = dynamic_cast<PropertyTreeWidgetItem*> (item);
	if (!prop_item)
		return;
	PropertyNode* node = prop_item->node();
	if (!node)
		return;
	PropertyValueNode<bool>* node_bool = dynamic_cast<PropertyValueNode<bool>*> (node);
	if (node_bool)
	{
		bool checked = prop_item->checkState (column) == Qt::Checked;
		if (checked != node_bool->read())
		{
			node_bool->write (checked);
			prop_item->read();
			item_selected();
		}
	}
}


void
PropertyEditor::update_item()
{
	PropertyNode* node = _property_tree_widget->selected_property_node();
	if (!node)
		return;
	TypedPropertyValueNode* val_node = dynamic_cast<TypedPropertyValueNode*> (node);
	if (!val_node)
		return;

	try {
		val_node->parse (_editable_value->text().toStdString());
		set_line_edit_color (_accepted_color);
		_accepted_blink_timer->start();
	}
	catch (SI::UnparsableValue const& e)
	{
		set_line_edit_color (_error_color);
	}
	catch (SI::UnsupportedUnit const& e)
	{
		set_line_edit_color (_error_color);
	}
}


void
PropertyEditor::reset_error()
{
	_accepted_blink_timer->stop();
	set_line_edit_color (_normal_color);
}

} // namespace Xefis

