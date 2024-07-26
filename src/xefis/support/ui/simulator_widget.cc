/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
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
#include "simulator_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/support/simulation/simulator.h>
#include <xefis/support/ui/paint_helper.h>

// Qt:
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QShortcut>

// Standard:
#include <cstddef>


namespace xf {

SimulatorWidget::SimulatorWidget (Simulator& simulator, QWidget* parent):
	QWidget (parent),
	_simulator (simulator)
{
	setWindowTitle("Xefis simulator");

	_rigid_body_viewer.emplace (this, RigidBodyViewer::AutoFPS);
	_rigid_body_viewer->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	_rigid_body_viewer->set_rigid_body_system (&_simulator.rigid_body_system());
	_rigid_body_viewer->set_redraw_callback ([this] (si::Time const frame_time) {
		_simulator.evolve (frame_time, 100_ms);
	});

	auto* sim_controls = new QWidget (this);
	sim_controls->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	auto* viewer_frame = new QFrame (this);
	viewer_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);

	auto* body_controls = new QLabel ("body controls", this);
	sim_controls->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	auto* start_stop_sim_button = new QPushButton ("Start/stop simulation", this);
	QObject::connect (start_stop_sim_button, &QPushButton::pressed, [this] {
		_rigid_body_viewer->toggle_pause();
	});

	auto* step_sim_button = new QPushButton ("Single step", this);
	QObject::connect (step_sim_button, &QPushButton::pressed, [this] {
		_rigid_body_viewer->step();
	});

	auto* show_configurator_button = new QPushButton ("Show machine config", this);
	QObject::connect (show_configurator_button, &QPushButton::pressed, [this] {
		if (_machine)
			_machine->show_configurator();
	});

	auto* sim_controls_layout = new QHBoxLayout (sim_controls);
	sim_controls_layout->setMargin (0);
	sim_controls_layout->addWidget (start_stop_sim_button);
	sim_controls_layout->addWidget (step_sim_button);
	sim_controls_layout->addWidget (show_configurator_button);

	auto* viewer_frame_layout = new QHBoxLayout (viewer_frame);
	viewer_frame_layout->addWidget (&*_rigid_body_viewer);
	viewer_frame_layout->setMargin (0);

	auto grid_layout = new QGridLayout (this);
	grid_layout->addWidget (sim_controls, 0, 0, 1, 2);
	grid_layout->addWidget (viewer_frame, 1, 0);
	grid_layout->addWidget (body_controls, 1, 1);

	auto const ph = PaintHelper (*this, palette(), font());
	resize (QSize (ph.em_pixels (80.0), ph.em_pixels (50.0)));
}

} // namespace xf

