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
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/simulation/devices/wing.h>
#include <xefis/support/ui/paint_helper.h>
#include <xefis/support/ui/rigid_body_viewer.h>
#include <xefis/support/ui/widget.h>

// Neutrino:
#include <neutrino/format.h>

// Qt:
#include <QLayout>
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

	_airfoil_spline_widget.emplace();
	_airfoil_spline_widget->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	_airfoil_spline_widget->setMinimumSize (ph.em_pixels (20), ph.em_pixels (10));// TODO

	_airfoil_frame.emplace();
	_airfoil_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);

	auto* airfoil_frame_layout = new QHBoxLayout (&*_airfoil_frame);
	airfoil_frame_layout->addWidget (&*_airfoil_spline_widget);
	airfoil_frame_layout->setMargin (0);

	_tool_box.emplace();
	_tool_box->addItem (new QLabel ("TODO", this), icons::body(), "Placement");
	_tool_box->addItem (new QLabel ("TODO", this), icons::body(), "Mass moments");
	_tool_box->addItem (new QLabel ("TODO", this), icons::body(), "Velocity moments");
	_tool_box->addItem (new QLabel ("TODO", this), icons::body(), "External force moments");
	_tool_box->addItem (new QLabel ("TODO", this), icons::body(), "Custom impulses");
	_tool_box->addItem (new QLabel ("Acceleration, kinetic energy, broken?", this), icons::body(), "Computed");

	auto* layout = new QVBoxLayout (this);
	layout->addWidget (top_strip);
	layout->addWidget (&*_airfoil_frame);
	layout->addWidget (create_basic_info_widget());
	layout->addWidget (&*_tool_box);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	setEnabled (false);
	refresh();
}


void
BodyEditor::edit (rigid_body::Body* body_to_edit)
{
	_edited_body = body_to_edit;
	refresh();
}


void
BodyEditor::refresh()
{
	if (_edited_body)
	{
		setEnabled (true);
		_body_label->setText (QString::fromStdString (_edited_body->label()));
		auto const translational_energy = neutrino::format_unit (_edited_body->translational_kinetic_energy().in<si::Joule>(), 6, "J");
		auto const rotational_energy = neutrino::format_unit (_edited_body->rotational_kinetic_energy().in<si::Joule>(), 6, "J");
		_translational_kinetic_energy->setText (QString::fromStdString (std::format ("Translational kinetic energy: {}", translational_energy)));
		_rotational_kinetic_energy->setText (QString::fromStdString (std::format ("Rotational kinetic energy: {}", rotational_energy)));

		if (auto* wing = dynamic_cast<sim::Wing*> (_edited_body))
		{
			auto const xy = []<class Value, class Space> (SpaceVector<Value, Space> const& vector3) {
				return PlaneVector<Value, Space> (vector3.x(), vector3.y());
			};

			auto const center_of_pressure_3d = wing->center_of_pressure() / wing->airfoil().chord_length();
			_airfoil_spline_widget->set_airfoil (wing->airfoil());
			_airfoil_spline_widget->set_center_of_pressure_position (xy (center_of_pressure_3d));
			_airfoil_spline_widget->set_lift_force (xy (wing->lift_force()));
			_airfoil_spline_widget->set_drag_force (xy (wing->drag_force()));
			_airfoil_spline_widget->set_pitching_moment (xy (wing->pitching_moment()));
			_airfoil_frame->show();
		}
		else
			_airfoil_frame->hide();
	}
	else
	{
		setEnabled (false);
		_body_label->setText ("–");
	}
}


QWidget*
BodyEditor::create_basic_info_widget()
{
	auto* widget = new QWidget (this);
	widget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	{
		_translational_kinetic_energy.emplace();
		_rotational_kinetic_energy.emplace();

		auto* layout = new QVBoxLayout (widget);
		layout->addWidget (&*_translational_kinetic_energy);
		layout->addWidget (&*_rotational_kinetic_energy);
	}

	return widget;
}
}

} // namespace xf

