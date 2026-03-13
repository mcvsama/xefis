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


ObservationWidget::ObservationWidget()
{
	_layout.setContentsMargins (0, 0, 0, 0);
}


ObservationWidget::ObservationWidget (rigid_body::Group* group):
	ObservationWidget()
{
	_group = group;
	init_specific_variants (_group);

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
	init_specific_variants (_body);

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
	init_specific_variants (_constraint);

	if (_constraint)
	{
		add_constraint_forces_observables();
		add_specific_observables();
	}
}


void
ObservationWidget::update_observed_values (rigid_body::Body const* planet_body)
{
	_planet_body = planet_body;
	fetch_values();

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
	auto const g = make_getter_maker (_basic_variables);

	auto group = add_group();
	group.add_observable ("Mass", g ([](BasicVariables const& basic_vars) {
		return nu::format_unit (basic_vars.mass_moments.mass().in<si::Gram>(), 6, "g");
	}));
	group.add_observable ("Translational kinetic energy", g ([](BasicVariables const& basic_vars) {
		return nu::format_unit (basic_vars.translational_kinetic_energy, 6);
	}));
	group.add_observable ("Rotational kinetic energy", g ([](BasicVariables const& basic_vars) {
		return nu::format_unit (basic_vars.rotational_kinetic_energy, 6);
	}));
	return group;
}


void
ObservationWidget::add_position_observables()
{
	auto const g = make_getter_maker (_position_variables);

	auto group = add_group();
	group.add_observable ("Latitude", g ([this] (PositionVariables const& position_vars) {
		return _planet_body ? std::format ("{:.6f}", position_vars.polar_location.lat().to<si::Degree>()) : "";
	}));
	group.add_observable ("Longitude:", g ([this] (PositionVariables const& position_vars) {
		return _planet_body ? std::format ("{:.6f}", position_vars.polar_location.lon().to<si::Degree>()) : "";
	}));
	group.add_observable ("AMSL height:", g ([this] (PositionVariables const& position_vars) {
		return _planet_body ? nu::format_unit (position_vars.polar_location.radius() - xf::kEarthMeanRadius, 6) : "";
	}));
}


void
ObservationWidget::add_velocity_observables()
{
	auto const g = make_getter_maker (_velocity_variables);

	auto group = add_group();
	group.add_observable ("Velocity", g ([](VelocityVariables const& velocity_vars) {
		return nu::format_unit (abs (velocity_vars.velocity_moments.velocity()), 3);
	}));
	group.add_observable ("Angular velocity", g ([](VelocityVariables const& velocity_vars) {
		return nu::format_unit (abs (velocity_vars.velocity_moments.angular_velocity()), 3);
	}));
}


void
ObservationWidget::add_constraint_forces_observables()
{
	_constraint_forces_variables = std::make_unique<ConstraintForcesVariables>();

	for (std::size_t body_index = 0; body_index < 2; ++body_index)
	{
		auto const g = make_getter_maker (_constraint_forces_variables);
		auto const force_getter = [body_index, g] (std::size_t axis) {
			return g ([body_index, axis] (ConstraintForcesVariables const& cf_vars) {
				return nu::format_unit (cf_vars.constraint_forces[body_index].force()[axis], 6);
			});
		};
		auto const torque_getter = [body_index, g] (std::size_t axis) {
			return g ([body_index, axis] (ConstraintForcesVariables const& cf_vars) {
				return nu::format_unit (cf_vars.constraint_forces[body_index].torque()[axis].in<si::NewtonMeter>(), 6, "Nm");
			});
		};

		auto group = add_group (body_index == 0 ? u8"Constraint forces: body 1" : u8"Constraint forces: body 2");

		group.add_observable ("Force X", force_getter (0));
		group.add_observable ("Force Y", force_getter (1));
		group.add_observable ("Force Z", force_getter (2));
		group.add_observable ("‖Force‖", g ([body_index] (ConstraintForcesVariables const& cf_vars) {
			return nu::format_unit (abs (cf_vars.constraint_forces[body_index].force()), 6);
		}));

		group.add_observable ("Torque X", torque_getter (0));
		group.add_observable ("Torque Y", torque_getter (1));
		group.add_observable ("Torque Z", torque_getter (2));
		group.add_observable ("‖Torque‖", g ([body_index] (ConstraintForcesVariables const& cf_vars) {
			return nu::format_unit (abs (cf_vars.constraint_forces[body_index].torque()).in<si::NewtonMeter>(), 6, "Nm");
		}));
	}
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
		auto const g = make_getter_maker (_aerodynamic_variables);

		auto group = add_group (u8"Aerodynamic variables");
		group.add_observable ("True air speed", g ([](AerodynamicVariables const& aero_vars) {
			return nu::format_unit (aero_vars.true_air_speed, 3);
		}));
		group.add_observable ("Static air temperature", g ([](AerodynamicVariables const& aero_vars) {
			return std::format ("{:.1f}", aero_vars.static_air_temperature.to<si::Celsius>());
		}));
		group.add_observable ("Air density", g ([](AerodynamicVariables const& aero_vars) {
			return nu::format_unit (aero_vars.air_density, 3);
		}));
		group.add_observable ("Dynamic viscosity:", g ([](AerodynamicVariables const& aero_vars) {
			return std::format ("{:.4g}", aero_vars.dynamic_viscosity);
		}));
		group.add_observable ("Reynolds number:", g ([](AerodynamicVariables const& aero_vars) {
			return std::format ("{:.0f}", *aero_vars.reynolds_number);
		}));
	}
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
ObservationWidget::fetch_values()
{
	fetch_basic_values();
	fetch_position_values();
	fetch_velocity_values();
	fetch_constraint_forces_values();
	fetch_specific_values();
}


void
ObservationWidget::fetch_basic_values()
{
	if (_body)
	{
		if (!_basic_variables)
			_basic_variables = std::make_unique<BasicVariables>();

		_basic_variables->mass_moments = _body->mass_moments<WorldSpace>();
		_basic_variables->translational_kinetic_energy = _body->translational_kinetic_energy();
		_basic_variables->rotational_kinetic_energy = _body->rotational_kinetic_energy();
	}
	else if (_group)
	{
		if (!_basic_variables)
			_basic_variables = std::make_unique<BasicVariables>();

		_basic_variables->mass_moments = _group->mass_moments();
		_basic_variables->translational_kinetic_energy = _group->translational_kinetic_energy();
		_basic_variables->rotational_kinetic_energy = _group->rotational_kinetic_energy();
	}
	else
		_basic_variables.reset();
}


void
ObservationWidget::fetch_position_values()
{
	if (_planet_body && (_body || _group))
	{
		if (!_position_variables)
			_position_variables = std::make_unique<PositionVariables>();

		auto const position = _body
			? _body->placement().position()
			: _basic_variables
				? _basic_variables->mass_moments.center_of_mass_position()
				: SpaceLength<WorldSpace> (math::zero);
		auto const position_on_planet = position - _planet_body->placement().position();
		// Assuming the planet is in ECEF orientation.
		_position_variables->polar_location = to_polar (math::coordinate_system_cast<ECEFSpace, void, WorldSpace, void> (position_on_planet));
	}
	else
		_position_variables.reset();
}


void
ObservationWidget::fetch_velocity_values()
{
	if (_planet_body && _body)
	{
		if (!_velocity_variables)
			_velocity_variables = std::make_unique<VelocityVariables>();

		_velocity_variables->velocity_moments = _body->velocity_moments<WorldSpace>() - _planet_body->velocity_moments<WorldSpace>();
	}
	else
		_velocity_variables.reset();
}


void
ObservationWidget::fetch_constraint_forces_values()
{
	if (_constraint)
	{
		auto optional_forces = _constraint->previous_computation_constraint_forces();

		if (optional_forces)
		{
			if (!_constraint_forces_variables)
				_constraint_forces_variables = std::make_unique<ConstraintForcesVariables>();

			_constraint_forces_variables->constraint_forces = *optional_forces;
		}
		else
			_constraint_forces_variables.reset();
	}
	else
		_constraint_forces_variables.reset();
}


void
ObservationWidget::fetch_specific_values()
{
	fetch_specific_values (_has_aerodynamic_parameters);
	// More in future if needed.
}


void
ObservationWidget::fetch_specific_values (HasAerodynamicParameters const* aerodynamic_object)
{
	if (aerodynamic_object)
	{
		if (auto const aerodynamic_parameters = aerodynamic_object->aerodynamic_parameters())
		{
			if (!_aerodynamic_variables)
				_aerodynamic_variables = std::make_unique<AerodynamicVariables>();

			auto const& air = aerodynamic_parameters->air;

			_aerodynamic_variables->true_air_speed = aerodynamic_parameters->true_air_speed;
			_aerodynamic_variables->static_air_temperature = air.temperature;
			_aerodynamic_variables->air_density = air.density;
			_aerodynamic_variables->dynamic_viscosity = air.dynamic_viscosity;
			_aerodynamic_variables->reynolds_number = aerodynamic_parameters->reynolds_number;
		}
		else
			_aerodynamic_variables.reset();
	}
	else
		_aerodynamic_variables.reset();
}

} // namespace xf
