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

#ifndef XEFIS__SUPPORT__UI__OBSERVATION_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__UI__OBSERVATION_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/properties/has_aerodynamic_parameters.h>
#include <xefis/support/simulation/rigid_body/group.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/simulation/rigid_body/constraint.h>
#include <xefis/utility/smoother.h>

// Qt:
#include <QGridLayout>
#include <QLabel>
#include <QWidget>

// Standard:
#include <cstddef>
#include <concepts>
#include <memory>
#include <string_view>
#include <type_traits>


namespace xf {

class ObservationWidget;


class ObservationWidgetGroup
{
  public:
	using Getter = std::function<std::string()>;
	using Setter = std::function<void (std::string_view)>;

  public:
	// Ctor
	explicit
	ObservationWidgetGroup (ObservationWidget& widget, QGridLayout& layout);

	void
	add_widget (QWidget&);

	QLabel&
	add_observable (std::string_view name, Getter, Setter = nullptr);

	QLabel&
	add_observable (std::string_view name, std::string& observed_string, Setter = nullptr);

  private:
	ObservationWidget*	_widget;
	QGridLayout*		_layout;
};


class ObservationWidget: public QWidget
{
	friend class ObservationWidgetGroup;

  public:
	using Getter = std::function<std::string()>;
	using Setter = std::function<void (std::string_view)>;

	struct Observable
	{
		QLabel*	value_label;
		Getter	get;
		Setter	set;
	};

  private:
	struct BasicVariables
	{
		MassMomentsAtArm<WorldSpace>	mass_moments;
		si::Energy						translational_kinetic_energy;
		si::Energy						rotational_kinetic_energy;
	};

	struct PositionVariables
	{
		si::LonLatRadius<> polar_location;
	};

	struct VelocityVariables
	{
		VelocityMoments<WorldSpace> velocity_moments;
	};

	struct ConstraintForcesVariables
	{
		rigid_body::ConstraintForces constraint_forces;
	};

	struct AerodynamicVariables
	{
		si::Velocity			true_air_speed;
		si::Temperature			static_air_temperature;
		si::Density				air_density;
		si::DynamicViscosity	dynamic_viscosity;
		ReynoldsNumber			reynolds_number;
	};

  public:
	// Ctor
	explicit
	ObservationWidget();

	// Ctor
	explicit
	ObservationWidget (rigid_body::Group*);

	// Ctor
	explicit
	ObservationWidget (rigid_body::Body*);

	// Ctor
	explicit
	ObservationWidget (rigid_body::Constraint*);

	// Dtor
	virtual
	~ObservationWidget() = default;

	/**
	 * Update values in the widget.
	 */
	virtual void
	update_observed_values (rigid_body::Body const* planet_body);

  protected:
	/**
	 * Add and return new group of observables.
	 */
	ObservationWidgetGroup
	add_group (std::u8string_view title = u8"");

	/**
	 * Add widget to the layout.
	 */
	void
	add_widget (QWidget&);

	/**
	 * Return the value QLabel.
	 */
	QLabel&
	add_observable (std::string_view name, Getter, Setter = nullptr);

	/**
	 * Return the value QLabel.
	 */
	QLabel&
	add_observable (std::string_view name, std::string& observed_string, Setter = nullptr);

  private:
	template<class T>
		void
		init_specific_variants (T const* observed_object)
		{
			if constexpr (std::is_polymorphic_v<T>)
				if (observed_object)
					_has_aerodynamic_parameters = dynamic_cast<HasAerodynamicParameters const*> (observed_object);
		}

	ObservationWidgetGroup
	add_basic_observables();

	void
	add_position_observables();

	void
	add_velocity_observables();

	void
	add_constraint_forces_observables();

	void
	add_specific_observables();

	void
	add_specific_observables (HasAerodynamicParameters const*);

	void
	add_widget (QWidget&, QGridLayout&);

	QLabel&
	add_observable (std::string_view name, Getter, Setter, QGridLayout&);

	/**
	 * Get values from observed body/group/whatever and cache them locally
	 * for later use in the widget.
	 */
	void
	fetch_values();

	void
	fetch_basic_values();

	void
	fetch_position_values();

	void
	fetch_velocity_values();

	void
	fetch_constraint_forces_values();

	void
	fetch_specific_values();

	void
	fetch_specific_values (HasAerodynamicParameters const*);

	template<class Variables>
		[[nodiscard]]
		static Getter
		make_getter_with_fallback (std::unique_ptr<Variables> const& variables, std::function<std::string (Variables const&)> getter)
		{
			return [&variables, getter] {
				if (variables)
					return getter (*variables);
				else
					return std::string ("–");
			};
		}

	template<class VariablesType>
		static auto
		make_getter_maker (std::unique_ptr<VariablesType>& ptr)
		{
			return [&ptr] (std::invocable<VariablesType> auto getter) {
				return make_getter_with_fallback<VariablesType> (ptr, getter);
			};
		}

  private:
	rigid_body::Group*				_group						{ nullptr };
	rigid_body::Body*				_body						{ nullptr };
	rigid_body::Constraint*			_constraint					{ nullptr };
	HasAerodynamicParameters const*	_has_aerodynamic_parameters	{ nullptr };
	QGridLayout						_layout						{ this };
	rigid_body::Body const*			_planet_body				{ nullptr };
	std::vector<Observable>			_observables;

	Smoother<float>								_load_factor_smoother		{ 100_ms, 10_ms };
	std::unique_ptr<BasicVariables>				_basic_variables;
	std::unique_ptr<PositionVariables>			_position_variables;
	std::unique_ptr<VelocityVariables>			_velocity_variables;
	std::unique_ptr<ConstraintForcesVariables>	_constraint_forces_variables;
	std::unique_ptr<AerodynamicVariables>		_aerodynamic_variables;
};

} // namespace xf

#endif
