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

	_tool_box.emplace();
	_tool_box->addItem (new QLabel ("TODO", this), icons::body(), "Placement");
	_tool_box->addItem (create_mass_moments_widget(), icons::body(), "Mass moments");
	_tool_box->addItem (new QLabel ("TODO", this), icons::body(), "Velocity moments");
	_tool_box->addItem (new QLabel ("TODO", this), icons::body(), "External force moments");
	_tool_box->addItem (new QLabel ("TODO", this), icons::body(), "Custom impulses");
	_tool_box->addItem (new QLabel ("Acceleration, kinetic energy, broken?", this), icons::body(), "Computed");

	_airfoil_info_widget = create_airfoil_info_widget (ph);

	auto* layout = new QVBoxLayout (this);
	layout->addWidget (top_strip);
	layout->addWidget (create_basic_info_widget());
	layout->addWidget (_airfoil_info_widget);
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
		auto const translational_energy = neutrino::format_unit (_edited_body->translational_kinetic_energy().in<si::Joule>(), 6, "J");
		auto const rotational_energy = neutrino::format_unit (_edited_body->rotational_kinetic_energy().in<si::Joule>(), 6, "J");

		setEnabled (true);
		_body_label->setText (QString::fromStdString (_edited_body->label()));
		_mass_value->setText (QString ("%1").arg (_edited_body->mass_moments<rigid_body::BodyCOM>().mass().in<si::Kilogram>()));
		_translational_kinetic_energy->setText (QString::fromStdString (std::format ("{}", translational_energy)));
		_rotational_kinetic_energy->setText (QString::fromStdString (std::format ("{}", rotational_energy)));

		refresh_wing_specific_data();
	}
	else
	{
		setEnabled (false);
		_body_label->setText ("–");
	}
}


void
BodyEditor::refresh_wing_specific_data()
{
	if (auto* wing = dynamic_cast<sim::Wing*> (_edited_body))
	{
		auto const not_computed = QString ("–");

		auto const set_wing_values = [this](QString const& value) {
			_true_air_speed->setText (value);
			_static_air_temperature->setText (value);
			_air_density->setText (value);
			_dynamic_viscosity->setText (value);
			_reynolds_number->setText (value);
		};

		auto const* rigid_body_system = _rigid_body_viewer.rigid_body_system();
		auto const* atmosphere = rigid_body_system ? rigid_body_system->atmosphere() : nullptr;

		if (atmosphere)
		{
			auto const aerodynamic_parameters = wing->airfoil_aerodynamic_parameters();

			if (aerodynamic_parameters)
			{
				auto const& air = aerodynamic_parameters->air;
				auto const& forces = aerodynamic_parameters->forces;

				_true_air_speed->setText (QString::fromStdString (std::format ("{:.3f}", aerodynamic_parameters->true_air_speed)));
				_static_air_temperature->setText (QString::fromStdString (std::format ("{:.1f}", air.temperature.to<si::Celsius>())));
				_air_density->setText (QString::fromStdString (std::format ("{:.3f}", air.density)));
				_dynamic_viscosity->setText (QString::fromStdString (std::format ("{:.4g}", air.dynamic_viscosity)));
				_reynolds_number->setText (QString::fromStdString (std::format ("{:.0f}", *aerodynamic_parameters->reynolds_number)));

				// Airfoil spline widget:
				{
					auto const xy = []<class Value, class Space> (SpaceVector<Value, Space> const& vector3) {
						return PlaneVector<Value, Space> (vector3.x(), vector3.y());
					};

					auto const center_of_pressure_3d = forces.center_of_pressure / wing->airfoil().chord_length();

					_airfoil_spline_widget->set_airfoil (wing->airfoil());
					_airfoil_spline_widget->set_center_of_pressure_position (xy (center_of_pressure_3d));
					_airfoil_spline_widget->set_lift_force (xy (forces.lift));
					_airfoil_spline_widget->set_drag_force (xy (forces.drag));
					_airfoil_spline_widget->set_pitching_moment (xy (forces.pitching_moment));
					_airfoil_info_widget->setVisible (true);
				}
			}
			else
				set_wing_values (not_computed);
		}
		else
			_airfoil_info_widget->setVisible (false);
	}
	else
		_airfoil_info_widget->setVisible (false);
}


QWidget*
BodyEditor::create_basic_info_widget()
{
	auto* widget = new QWidget (this);
	widget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	_mass_value.emplace(); // TODO changeable
	_translational_kinetic_energy.emplace();
	_rotational_kinetic_energy.emplace();

	auto* layout = new QGridLayout (widget);
	layout->addWidget (new QLabel ("Mass:"), 0, 0);
	layout->addWidget (&*_mass_value, 0, 1);
	layout->addWidget (new QLabel ("Translational kinetic energy:"), 1, 0);
	layout->addWidget (&*_translational_kinetic_energy, 1, 1);
	layout->addWidget (new QLabel ("Rotational kinetic energy:"), 2, 0);
	layout->addWidget (&*_rotational_kinetic_energy, 2, 1);

	return widget;
}


QWidget*
BodyEditor::create_airfoil_info_widget (PaintHelper const& ph)
{
	auto* widget = new QWidget (this);
	widget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	_airfoil_spline_widget.emplace();
	_airfoil_spline_widget->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	_airfoil_spline_widget->setMinimumSize (ph.em_pixels (20), ph.em_pixels (10));// TODO

	_airfoil_frame.emplace();
	_airfoil_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);

	_true_air_speed.emplace();
	_static_air_temperature.emplace();
	_air_density.emplace();
	_dynamic_viscosity.emplace();
	_reynolds_number.emplace();

	auto* airfoil_frame_layout = new QHBoxLayout (&*_airfoil_frame);
	airfoil_frame_layout->addWidget (&*_airfoil_spline_widget);
	airfoil_frame_layout->setMargin (0);

	int row = 0;
	auto* layout = new QGridLayout (widget);
	layout->addWidget (&*_airfoil_frame, row++, 0, 1, 2);
	layout->addWidget (new QLabel ("True air speed:"), row, 0);
	layout->addWidget (&*_true_air_speed, row++, 1);
	layout->addWidget (new QLabel ("Static air temperature:"), row, 0);
	layout->addWidget (&*_static_air_temperature, row++, 1);
	layout->addWidget (new QLabel ("Air density:"), row, 0);
	layout->addWidget (&*_air_density, row++, 1);
	layout->addWidget (new QLabel ("Dynamic viscosity:"), row, 0);
	layout->addWidget (&*_dynamic_viscosity, row++, 1);
	layout->addWidget (new QLabel ("Reynolds number:"), row, 0);
	layout->addWidget (&*_reynolds_number, row++, 1);

	return widget;
}


QWidget*
BodyEditor::create_mass_moments_widget()
{
	auto* widget = new QWidget (this);
	widget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	_mass_value.emplace ("–");
	_mass_value->setAlignment (Qt::AlignRight);

	// TODO add_editable (math::Matrix<...>) // Depending on size works as vector or matrix or whatever
	// TODO add_editable (Scalar)
	// ...
	auto* layout = new QGridLayout (widget);
	layout->addWidget (new QLabel ("Mass", widget), 0, 0);
	layout->addWidget (&*_mass_value, 0, 1);
	layout->addWidget (new QLabel ("kg", widget), 0, 2);

	return widget;
}

} // namespace xf

