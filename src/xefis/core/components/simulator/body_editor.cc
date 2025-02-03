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
#include <xefis/support/ui/rigid_body_viewer.h>
#include <xefis/support/ui/widget.h>

// Qt:
#include <QLayout>

// Standard:
#include <cstddef>


namespace xf {

BodyEditor::BodyEditor (QWidget* parent, RigidBodyViewer& viewer):
	QWidget (parent),
	_rigid_body_viewer (viewer)
{
	auto const ph = PaintHelper (*this);

	auto [top_strip, top_label] = Widget::create_colored_strip_label ("–", Qt::darkGreen, Qt::AlignBottom, this);
	top_strip->setMinimumWidth (ph.em_pixels_int (25));
	_body_label = top_label;

	auto* layout = new QVBoxLayout (this);
	layout->addWidget (top_strip);
	layout->addLayout (&_edited_body_widget_layout);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	setEnabled (false);
	refresh();
}


void
BodyEditor::edit (rigid_body::Body* body_to_edit)
{
	_edited_body = body_to_edit;
	_edited_body_widget.reset();

	if (_edited_body)
	{
		if (auto* has_observation_widget = dynamic_cast<HasObservationWidget*> (_edited_body))
			_edited_body_widget = has_observation_widget->create_observation_widget();
		else
			_edited_body_widget = std::make_unique<ObservationWidget> (_edited_body);

		if (_edited_body_widget)
			_edited_body_widget_layout.addWidget (_edited_body_widget.get());
	}

	refresh();
}


void
BodyEditor::refresh()
{
	if (_edited_body_widget)
		_edited_body_widget->update_observed_values (_rigid_body_viewer.planet());

	if (_edited_body)
	{
		setEnabled (true);
		_body_label->setText (QString::fromStdString (_edited_body->label()));
	}
	else
	{
		setEnabled (false);
		_body_label->setText ("–");
	}
}

} // namespace xf

