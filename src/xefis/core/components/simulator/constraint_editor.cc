/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Local:
#include "constraint_editor.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/base/icons.h>
#include <xefis/support/ui/paint_helper.h>
#include <xefis/support/ui/widget.h>

// Qt:
#include <QLayout>
#include <QPushButton>

// Standard:
#include <cstddef>


namespace xf {

ConstraintEditor::ConstraintEditor (QWidget* parent):
	QWidget (parent)
{
	auto const ph = PaintHelper (*this);

	auto [top_strip, top_label] = Widget::create_colored_strip_label ("–", QColor (0xff, 0x8c, 0), Qt::AlignBottom, this);
	top_strip->setMinimumWidth (ph.em_pixels_int (25));
	_constraint_label = top_label;

	auto* layout = new QVBoxLayout (this);
	layout->addWidget (top_strip);
	layout->addWidget (create_basic_info_widget());
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	setEnabled (false);
	refresh();
}


void
ConstraintEditor::edit (rigid_body::Constraint* constraint_to_edit)
{
	_edited_constraint = constraint_to_edit;
	refresh();
}


void
ConstraintEditor::refresh()
{
	if (_edited_constraint)
	{
		setEnabled (true);
		_constraint_label->setText (QString::fromStdString (_edited_constraint->label()));
	}
	else
	{
		setEnabled (false);
		_constraint_label->setText ("–");
	}
}


QWidget*
ConstraintEditor::create_basic_info_widget()
{
	auto* basic_info = new QWidget (this);
	basic_info->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);
	return basic_info;
}

} // namespace xf

