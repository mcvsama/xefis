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

#ifndef XEFIS__CORE__COMPONENTS__SIMULATOR__SIMULATOR_WIDGET_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__SIMULATOR__SIMULATOR_WIDGET_H__INCLUDED

// Local:
#include "bodies_tree.h"
#include "group_editor.h"
#include "body_editor.h"
#include "constraint_editor.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/base/icons.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/simulation/simulator.h>
#include <xefis/support/ui/rigid_body_viewer.h>

// Qt:
#include <QIcon>
#include <QStackedWidget>
#include <QWidget>

// Standard:
#include <cstddef>
#include <optional>


namespace xf {

class Machine;

/**
 * Widget for Simulator.
 * Shows rigid_body::System in a window.
 * Allows adding/configuring ob bodies and constraints and configure
 * electrical network, too.
 */
class SimulatorWidget: public QWidget
{
  public:
	// Ctor
	explicit
	SimulatorWidget (Simulator&, QWidget* parent);

	/**
	 * Set related machine. Used to show configurator widget when pressing Esc.
	 * Pass nullptr to unset.
	 */
	void
	set_machine (Machine* machine);

	/**
	 * Sets the followed group in the internal RigidBodyViewer.
	 */
	void
	set_followed (rigid_body::Group const&) noexcept;

	/**
	 * Sets the followed body in the internal RigidBodyViewer.
	 */
	void
	set_followed (rigid_body::Body const&) noexcept;

	/**
	 * Sets the planet body in the internal RigidBodyViewer.
	 */
	void
	set_planet (rigid_body::Body const* planet_body) noexcept
		{ _rigid_body_viewer->set_planet (planet_body); }

  private:
	[[nodiscard]]
	QWidget*
	make_viewer_widget();

	[[nodiscard]]
	QWidget*
	make_simulation_controls();

	[[nodiscard]]
	QWidget*
	make_body_controls();

	void
	update_editor_for (QTreeWidgetItem*);

	void
	update_simulation_time_label();

	void
	update_simulation_performance_label();

  private:
	Machine*						_machine					{ nullptr };
	Simulator&						_simulator;
	std::optional<RigidBodyViewer>	_rigid_body_viewer;
	// Warning: QStackedWidget deletes widgets added to it in its destructor:
	std::optional<QStackedWidget>	_editors_stack;
	std::optional<GroupEditor>		_group_editor;
	std::optional<BodyEditor>		_body_editor;
	std::optional<ConstraintEditor>	_constraint_editor;
	std::optional<BodiesTree>		_bodies_tree;
	std::optional<QLabel>			_simulation_time_label;
	std::optional<QLabel>			_simulation_performance_value_label;
	QIcon							_start_icon					{ icons::start() };
	QIcon							_pause_icon					{ icons::pause() };
	float							_simulation_speed			{ 1.0f };
	float							_last_finite_performance	{ 1.0f };
};


inline void
SimulatorWidget::set_machine (Machine* const machine)
{
	_machine = machine;
	_rigid_body_viewer->set_machine (machine);
}


inline void
SimulatorWidget::set_followed (rigid_body::Group const& followed_group) noexcept
{
	_rigid_body_viewer->set_followed (followed_group);
	_bodies_tree->refresh();
}


inline void
SimulatorWidget::set_followed (rigid_body::Body const& followed_body) noexcept
{
	_rigid_body_viewer->set_followed (followed_body);
	_bodies_tree->refresh();
}

} // namespace xf

#endif

