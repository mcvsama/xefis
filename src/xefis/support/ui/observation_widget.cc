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

// Local:
#include "observation_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/constants.h>

// Neutrino:
#include <neutrino/format.h>
#include <neutrino/qt/qstring.h>

// Qt:
#include <QGridLayout>
#include <QGroupBox>

// Standard:
#include <cstddef>


namespace xf {

ObservationWidgetGroup::ObservationWidgetGroup (ObservationWidget& widget, QGridLayout& layout):
	_widget (&widget),
	_layout (&layout)
{ }


void
ObservationWidgetGroup::add_widget (QWidget& widget)
{
	_widget->add_widget (widget, *_layout);
}


QLabel&
ObservationWidgetGroup::add_observable (std::string_view const name, Getter const getter, Setter const setter)
{
	return _widget->add_observable (name, getter, setter, *_layout);
}


QLabel&
ObservationWidgetGroup::add_observable (std::string_view const name, std::string& observed_string, Setter const setter)
{
	return add_observable (name, [&observed_string]() { return observed_string; }, setter);
}


ObservationWidget::ObservationWidget()
{
	_layout.setMargin (0);
}


ObservationWidget::ObservationWidget (rigid_body::Group* group):
	ObservationWidget()
{
	_group = group;

	if (_group)
	{
		// Basic information:
		{
			auto group = add_group();
			group.add_observable ("Mass", [this]() {
				return neutrino::format_unit (_group->mass_moments().mass().in<si::Kilogram>() * 1000.0, 6, "g");
			});
		}
	}
}


ObservationWidget::ObservationWidget (rigid_body::Body* body):
	ObservationWidget()
{
	_body = body;

	if (_body)
	{
		// Basic information:
		{
			auto group = add_group();
			group.add_observable ("Mass", [this]() {
				return neutrino::format_unit (_body->mass_moments<BodyCOM>().mass().in<si::Kilogram>() * 1000.0, 6, "g");
			});
			group.add_observable ("Translational kinetic energy", [this]() {
				return neutrino::format_unit (_body->translational_kinetic_energy().in<si::Joule>(), 6, "J");
			});
			group.add_observable ("Rotational kinetic energy", [this]() {
				return neutrino::format_unit (_body->rotational_kinetic_energy().in<si::Joule>(), 6, "J");
			});
			group.add_observable ("Load factor", [this]() {
				// TODO low pass filter on load factor:
				auto const acceleration = _body->acceleration_moments_except_gravity<BodyCOM>().acceleration();
				// Wing's down in BodyCOM (airfoil coordinates) is negative Y, so use .y():
				return std::format ("{:.2f}", acceleration.y() / xf::kStdGravitationalAcceleration);
			});
		}

		// Position:
		{
			auto group = add_group (u8"Position");
			group.add_observable ("Latitude", [this]() {
				return _planet_body ? std::format ("{:.6f}", _polar_location.lat().to<si::Degree>()) : "";
			});
			group.add_observable ("Longitude:", [this]() {
				return _planet_body ? std::format ("{:.6f}", _polar_location.lon().to<si::Degree>()) : "";
			});
			group.add_observable ("AMSL height:", [this]() {
				return _planet_body ? std::format ("{:.3f}", _polar_location.radius() - xf::kEarthMeanRadius) : "";
			});
		}

		// Velocities:
		{
			auto group = add_group (u8"Velocities");
			group.add_observable ("Velocity", [this]() {
				return std::format ("{:.3f}", abs (_velocity_moments.velocity()));
			});
			group.add_observable ("Angular velocity", [this]() {
				return std::format ("{:.3f}", abs (_velocity_moments.angular_velocity()));
			});
		}

		// TODO
		// (new QLabel ("TODO", this), icons::body(), "External force moments");
		// (new QLabel ("TODO", this), icons::body(), "Custom impulses");
		// (new QLabel ("Acceleration, kinetic energy, broken?", this), icons::body(), "Computed");
	}
}


ObservationWidget::ObservationWidget (rigid_body::Constraint* constraint):
	ObservationWidget()
{
	_constraint = constraint;

	if (_constraint)
	{
		// TODO last calculation of constraint forces
	}
}


void
ObservationWidget::update_observed_values (rigid_body::Body const* planet_body)
{
	_planet_body = planet_body;

	if (_planet_body && _body)
	{
		auto const position_on_planet = _body->placement().position() - planet_body->placement().position();
		// Assuming the planet is in ECEF orientation.
		_polar_location = xf::polar (math::coordinate_system_cast<ECEFSpace, void> (position_on_planet));
		_velocity_moments = _body->velocity_moments<WorldSpace>() - planet_body->velocity_moments<WorldSpace>();
	}

	for (auto& observable: _observables)
	{
		auto const text = observable.get ? observable.get() : "–";
		observable.value_label->setText (QString::fromStdString (text));

		// TODO handle observable.set
	}
}


ObservationWidgetGroup
ObservationWidget::add_group (std::u8string_view const title)
{
	auto* group_box = new QGroupBox (to_qstring (title), this);
	auto* group_box_layout = new QGridLayout (group_box);
	auto const row = _layout.rowCount();
	_layout.addWidget (group_box, row, 0, 1, 2);
	return ObservationWidgetGroup (*this, *group_box_layout);
}


void
ObservationWidget::add_widget (QWidget& widget)
{
	add_widget (widget, _layout);
}


QLabel&
ObservationWidget::add_observable (std::string_view const name, Getter const getter, Setter const setter)
{
	return add_observable (name, getter, setter, _layout);
}


QLabel&
ObservationWidget::add_observable (std::string_view const name, std::string& observed_string, Setter const setter)
{
	return add_observable (name, [&observed_string]() { return observed_string; }, setter);
}


void
ObservationWidget::add_widget (QWidget& widget, QGridLayout& layout)
{
	auto const row = layout.rowCount();
	layout.addWidget (&widget, row, 0, 1, 2);
}


QLabel&
ObservationWidget::add_observable (std::string_view const name, Getter const getter, Setter const setter, QGridLayout& layout)
{
	auto* value_label = new QLabel ("–");
	_observables.emplace_back (value_label, getter, setter);

	auto row = layout.rowCount();
	layout.addWidget (new QLabel (QString::fromStdString (std::string (name))), row, 0);
	layout.addWidget (value_label, row, 1);

	return *value_label;
}


std::unique_ptr<ObservationWidget>
HasObservationWidget::create_observation_widget()
{
	if (rigid_body::Group* group = dynamic_cast<rigid_body::Group*> (this))
		return std::make_unique<ObservationWidget> (group);
	else if (rigid_body::Body* body = dynamic_cast<rigid_body::Body*> (this))
		return std::make_unique<ObservationWidget> (body);
	else if (rigid_body::Constraint* constraint = dynamic_cast<rigid_body::Constraint*> (this))
		return std::make_unique<ObservationWidget> (constraint);
	else
		return std::make_unique<ObservationWidget>();
}

} // namespace xf

