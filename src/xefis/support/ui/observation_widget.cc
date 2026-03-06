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


void
ObservationWidget::AerodynamicVariables::reset()
{
	for (auto* str: { &true_air_speed, &static_air_temperature, &air_density, &dynamic_viscosity, &reynolds_number })
		*str = "–";
}


ObservationWidget::ObservationWidget()
{
	_layout.setContentsMargins (0, 0, 0, 0);
}


ObservationWidget::ObservationWidget (rigid_body::Group* group):
	ObservationWidget()
{
	_group = group;
	init_variants (_group);

	if (_group)
	{
		add_basic_observables();
		add_position_observables();
		add_specific_observables();
	}
}


ObservationWidget::ObservationWidget (rigid_body::Body* body):
	ObservationWidget()
{
	_body = body;
	init_variants (_body);

	if (_body)
	{
		auto basic_info_group = add_basic_observables();
		basic_info_group.add_observable ("Load factor", [this, prev_time = nu::utc_now()]() mutable {
			auto const now = nu::utc_now();
			auto const dt = now - prev_time;
			prev_time = now;

			auto const acceleration = _body->acceleration_moments_except_gravity<BodyCOM>().acceleration();
			// Wing's down in BodyCOM (airfoil coordinates) is negative Y, so use .y():
			auto const load_factor = acceleration.y() / xf::kStdGravitationalAcceleration;
			return std::format ("{:.2f}", _load_factor_smoother (load_factor, dt));
		});

		add_position_observables();
		add_velocity_observables();
		add_specific_observables();
	}
}


ObservationWidget::ObservationWidget (rigid_body::Constraint* constraint):
	ObservationWidget()
{
	_constraint = constraint;
	init_variants (_constraint);

	if (_constraint)
	{
		// TODO display last calculation of constraint forces
		add_specific_observables();
	}
}


void
ObservationWidget::update_observed_values (rigid_body::Body const* planet_body)
{
	_planet_body = planet_body;

	if (_body)
	{
		_mass_moments = _body->mass_moments<WorldSpace>();
		_translational_kinetic_energy = _body->translational_kinetic_energy();
		_rotational_kinetic_energy = _body->rotational_kinetic_energy();
	}
	else if (_group)
	{
		_mass_moments = _group->mass_moments();
		_translational_kinetic_energy = _group->translational_kinetic_energy();
		_rotational_kinetic_energy = _group->rotational_kinetic_energy();
	}

	if (_planet_body && (_body || _group))
	{
		auto const position = _body ? _body->placement().position() : _mass_moments.center_of_mass_position();
		auto const position_on_planet = position - planet_body->placement().position();
		// Assuming the planet is in ECEF orientation.
		_polar_location = to_polar (math::coordinate_system_cast<ECEFSpace, void, WorldSpace, void> (position_on_planet));

		if (_body)
			_velocity_moments = _body->velocity_moments<WorldSpace>() - planet_body->velocity_moments<WorldSpace>();
	}

	update_specific_values();

	for (auto& observable: _observables)
	{
		auto const text = observable.get ? observable.get() : "–";
		observable.value_label->setText (nu::to_qstring (text));

		// TODO handle observable.set
	}
}


ObservationWidgetGroup
ObservationWidget::add_group (std::u8string_view const title)
{
	auto* group_box = new QGroupBox (nu::to_qstring (title), this);
	auto* group_box_layout = new QGridLayout (group_box);
	auto const row = _layout.rowCount();
	_layout.addWidget (group_box, row, 0, 1, 2);
	return ObservationWidgetGroup (*this, *group_box_layout);
}


ObservationWidgetGroup
ObservationWidget::add_basic_observables()
{
	auto group = add_group();

	group.add_observable ("Mass", [this]() {
		return nu::format_unit (_mass_moments.mass().in<si::Gram>(), 6, "g");
	});
	group.add_observable ("Translational kinetic energy", [this]() {
		return nu::format_unit (_translational_kinetic_energy.in<si::Joule>(), 6, "J");
	});
	group.add_observable ("Rotational kinetic energy", [this]() {
		return nu::format_unit (_rotational_kinetic_energy.in<si::Joule>(), 6, "J");
	});

	return group;
}


void
ObservationWidget::add_position_observables()
{
	auto group = add_group();
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


void
ObservationWidget::add_velocity_observables()
{
	auto group = add_group();
	group.add_observable ("Velocity", [this]() {
		return std::format ("{:.3f}", abs (_velocity_moments.velocity()));
	});
	group.add_observable ("Angular velocity", [this]() {
		return std::format ("{:.3f}", abs (_velocity_moments.angular_velocity()));
	});
}


void
ObservationWidget::add_specific_observables()
{
	add_specific_observables (_has_aerodynamic_parameters);
	// More in future if needed.
}


void
ObservationWidget::add_specific_observables (HasAerodynamicParameters const* aerodynamic_object)
{
	if (aerodynamic_object)
	{
		if (!_aerodynamic_variables)
		{
			_aerodynamic_variables = std::make_unique<AerodynamicVariables>();

			auto group = add_group (u8"Aerodynamic variables");
			group.add_observable ("True air speed", _aerodynamic_variables->true_air_speed);
			group.add_observable ("Static air temperature", _aerodynamic_variables->static_air_temperature);
			group.add_observable ("Air density", _aerodynamic_variables->air_density);
			group.add_observable ("Dynamic viscosity:", _aerodynamic_variables->dynamic_viscosity);
			group.add_observable ("Reynolds number:", _aerodynamic_variables->reynolds_number);
		}

		// Initial read:
		update_specific_values (aerodynamic_object);
	}
	else
		_aerodynamic_variables.reset();
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
	layout.addWidget (new QLabel (nu::to_qstring (std::string (name))), row, 0);
	layout.addWidget (value_label, row, 1);

	return *value_label;
}


void
ObservationWidget::update_specific_values()
{
	update_specific_values (_has_aerodynamic_parameters);
	// More in future if needed.
}


void
ObservationWidget::update_specific_values (HasAerodynamicParameters const* aerodynamic_object)
{
	if (aerodynamic_object)
	{
		if (auto const aerodynamic_parameters = aerodynamic_object->aerodynamic_parameters())
		{
			auto const& air = aerodynamic_parameters->air;

			_aerodynamic_variables->true_air_speed = std::format ("{:.3f}", aerodynamic_parameters->true_air_speed);
			_aerodynamic_variables->static_air_temperature = std::format ("{:.1f}", air.temperature.to<si::Celsius>());
			_aerodynamic_variables->air_density = std::format ("{:.3f}", air.density);
			_aerodynamic_variables->dynamic_viscosity = std::format ("{:.4g}", air.dynamic_viscosity);
			_aerodynamic_variables->reynolds_number = std::format ("{:.0f}", *aerodynamic_parameters->reynolds_number);
		}
		else
			_aerodynamic_variables->reset();
	}
}

} // namespace xf
