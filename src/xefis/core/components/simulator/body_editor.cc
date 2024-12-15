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
#include <xefis/base/icons.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/simulation/devices/wing.h>
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/ui/paint_helper.h>
#include <xefis/support/ui/rigid_body_viewer.h>
#include <xefis/support/ui/widget.h>

// Qt:
#include <QLayout>
#include <QGridLayout>
#include <QPushButton>

// Standard:
#include <cstddef>


namespace xf {

BodyEditor::BodyEditor (QWidget* parent, RigidBodyViewer& viewer):
	QWidget (parent),
	_rigid_body_viewer (viewer)
{
	auto const ph = PaintHelper (*this);

	auto [top_strip, top_label] = Widget::create_colored_strip_label ("–", Qt::blue, Qt::AlignBottom, this);
	top_strip->setMinimumWidth (ph.em_pixels_int (25));
	_body_label = top_label;

	_tool_box.addItem (create_position_widget(), icons::body(), "Position");
	_tool_box.addItem (new QLabel ("TODO", this), icons::body(), "Velocity moments");
	_tool_box.addItem (new QLabel ("TODO", this), icons::body(), "External force moments");
	_tool_box.addItem (new QLabel ("TODO", this), icons::body(), "Custom impulses");
	_tool_box.addItem (new QLabel ("Acceleration, kinetic energy, broken?", this), icons::body(), "Computed");

	_layout.addWidget (top_strip);
	_layout.addLayout (&_edited_body_widget_layout);
	_layout.addWidget (&_tool_box);
	_layout.addItem (new QSpacerItem (0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

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
		{
			_edited_body_widget = has_observation_widget->create_observation_widget();

			if (_edited_body_widget)
				_edited_body_widget_layout.addWidget (_edited_body_widget.get());
		}
	}

	refresh();
}


void
BodyEditor::refresh()
{
	if (_edited_body_widget)
		_edited_body_widget->update_observed_values();

	if (_edited_body)
	{
		setEnabled (true);
		_body_label->setText (QString::fromStdString (_edited_body->label()));

		if (auto const* planet_body = _rigid_body_viewer.planet())
		{
			auto const position_on_planet = _edited_body->placement().position() - planet_body->placement().position();
			// Assuming the planet is in ECEF orientation.
			auto const polar_location = xf::polar (math::coordinate_system_cast<ECEFSpace, void> (position_on_planet));
			auto const velocity_moments = _edited_body->velocity_moments<WorldSpace>() - planet_body->velocity_moments<WorldSpace>();

			_longitude.setText (QString::fromStdString (std::format ("{:.6f}", polar_location.lon().to<si::Degree>())));
			_latitude.setText (QString::fromStdString (std::format ("{:.6f}", polar_location.lat().to<si::Degree>())));
			_altitude_amsl.setText (QString::fromStdString (std::format ("{:.3f}", polar_location.radius() - xf::kEarthMeanRadius)));
			_velocity.setText (QString::fromStdString (std::format ("{:.3f}", abs (velocity_moments.velocity()))));
			_angular_velocity.setText (QString::fromStdString (std::format ("{:.3f}", abs (velocity_moments.angular_velocity()))));
		}
		else
		{
			for (auto* w: { &_latitude, &_longitude, &_altitude_amsl, &_velocity, &_angular_velocity })
				w->setText ("–");
		}
	}
	else
	{
		setEnabled (false);
		_body_label->setText ("–");
	}
}


QWidget*
BodyEditor::create_position_widget()
{
	auto* widget = new QWidget (this);
	widget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	auto row = 0;
	auto* layout = new QGridLayout (widget);
	layout->addWidget (new QLabel ("Latitude:"), row, 0);
	layout->addWidget (&_latitude, row++, 1);
	layout->addWidget (new QLabel ("Longitude:" ), row, 0);
	layout->addWidget (&_longitude, row++, 1);
	layout->addWidget (new QLabel ("AMSL height:"), row, 0);
	layout->addWidget (&_altitude_amsl, row++, 1);
	layout->addWidget (new QLabel ("Velocity:"), row, 0);
	layout->addWidget (&_velocity, row++, 1);
	layout->addWidget (new QLabel ("Angular velocity:"), row, 0);
	layout->addWidget (&_angular_velocity, row++, 1);

	return widget;
}

} // namespace xf

