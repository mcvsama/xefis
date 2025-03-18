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
#include <memory>
#include <string_view>


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
	ObservationWidgetGroup
	add_basic_observables();

	void
	add_position_observables();

	void
	add_velocity_observables();

	void
	add_widget (QWidget&, QGridLayout&);

	QLabel&
	add_observable (std::string_view name, Getter, Setter, QGridLayout&);

  private:
	rigid_body::Group*				_group					{ nullptr };
	rigid_body::Body*				_body					{ nullptr };
	rigid_body::Constraint*			_constraint				{ nullptr };
	QGridLayout						_layout					{ this };
	rigid_body::Body const*			_planet_body			{ nullptr };
	std::vector<Observable>			_observables;
	// Typical observables:
	si::Energy						_translational_kinetic_energy;
	si::Energy						_rotational_kinetic_energy;
	si::LonLatRadius				_polar_location;
	VelocityMoments<WorldSpace>		_velocity_moments;
	MassMomentsAtArm<WorldSpace>	_mass_moments;
	Smoother<float>					_load_factor_smoother	{ 100_ms, 10_ms };
};


class HasObservationWidget
{
  public:
	// Dtor
	virtual
	~HasObservationWidget() = default;

	[[nodiscard]]
	virtual std::unique_ptr<ObservationWidget>
	create_observation_widget();
};

} // namespace xf

#endif

