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

#ifndef XEFIS__SUPPORT__UI__SIMULATOR_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__UI__SIMULATOR_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/simulation/evolver.h>
#include <xefis/support/ui/rigid_body_viewer.h>

// Qt:
#include <QIcon>
#include <QWidget>

// Standard:
#include <cstddef>
#include <optional>


namespace xf {

class Machine;
class RigidBodyViewer;
class Simulator;

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
	 * Sets the followed body in the internal RigidBodyViewer.
	 */
	void
	set_followed_body (rigid_body::Body const* followed_body) noexcept
		{ _rigid_body_viewer->set_followed_body (followed_body); }

	/**
	 * Sets the planet body in the internal RigidBodyViewer.
	 */
	void
	set_planet (rigid_body::Body const* planet_body) noexcept
		{ _rigid_body_viewer->set_planet (planet_body); }

	/**
	 * Sets redraw callback on the internal RigidBodyViewer.
	 */
	void
	set_redraw_callback (RigidBodyViewer::OnRedraw const on_redraw = {})
		{ _rigid_body_viewer->set_redraw_callback (on_redraw); }

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
	populate_with_bodies_and_constraints (QTreeWidget&, rigid_body::System&);

  private:
	Machine*						_machine				{ nullptr };
	Simulator&						_simulator;
	std::optional<RigidBodyViewer>	_rigid_body_viewer;
	QIcon							_start_icon				{ resources::icons16::start() };
	QIcon							_pause_icon				{ resources::icons16::pause() };
	QIcon							_followed_body_icon;
};


inline void
SimulatorWidget::set_machine (Machine* const machine)
{
	_machine = machine;
	_rigid_body_viewer->set_machine (machine);
}

} // namespace xf

#endif

