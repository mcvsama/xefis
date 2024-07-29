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
#include "body_editor.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/paint_helper.h>
#include <xefis/support/ui/widget.h>

// Qt:
#include <QLabel>
#include <QLayout>

// Standard:
#include <cstddef>


namespace xf {

BodyEditor::BodyEditor (QWidget* parent):
	QWidget (parent)
{
	auto const ph = PaintHelper (*this);

	auto [top_strip, top_label] = Widget::create_colored_strip_label ("abc", Qt::blue, Qt::AlignBottom, this);
	top_strip->setMinimumWidth (ph.em_pixels_int (20));
	_body_label = top_label;

	auto* layout = new QVBoxLayout (this);
	layout->addWidget (top_strip);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}


void
BodyEditor::edit_body (rigid_body::Body* body_to_edit)
{
	_edited_body = body_to_edit;
	refresh();
}


void
BodyEditor::refresh()
{
	if (_edited_body)
		_body_label->setText (QString::fromStdString (_edited_body->label()));
}

} // namespace xf

