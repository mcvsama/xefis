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
#include "bodies_tree.h"
#include "body_editor.h"
#include "body_item.h"
#include "simulator_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/support/simulation/simulator.h>
#include <xefis/support/ui/paint_helper.h>

// Qt:
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QShortcut>
#include <QSplitter>

// Standard:
#include <cstddef>


namespace xf {

SimulatorWidget::SimulatorWidget (Simulator& simulator, QWidget* parent):
	QWidget (parent),
	_simulator (simulator)
{
	setWindowTitle("Xefis simulator");
	auto const ph = PaintHelper (*this);

	auto* splitter = new QSplitter (this);
	splitter->addWidget (make_viewer_widget());
	splitter->addWidget (make_body_controls());
	splitter->setHandleWidth (ph.em_pixels (0.5));
	splitter->setStretchFactor (0, 4);
	splitter->setStretchFactor (1, 2);
	splitter->setSizes ({ ph.em_pixels_int (40), ph.em_pixels_int (20) });

	auto* layout = new QVBoxLayout (this);
	layout->addWidget (make_simulation_controls());
	layout->addWidget (splitter);

	resize (QSize (ph.em_pixels (80.0), ph.em_pixels (40.0)));
}


QWidget*
SimulatorWidget::make_viewer_widget()
{
	_rigid_body_viewer.emplace (this, RigidBodyViewer::AutoFPS);
	_rigid_body_viewer->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	_rigid_body_viewer->set_rigid_body_system (&_simulator.rigid_body_system());
	_rigid_body_viewer->set_redraw_callback ([this] (std::optional<si::Time> const simulation_time) {
		if (simulation_time)
			_simulator.evolve (*simulation_time, 100_ms);
		else
			_simulator.evolve (1);

		update_simulation_time_label();
		_body_editor->refresh();
	});

	auto* viewer_frame = new QFrame (this);
	viewer_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	viewer_frame->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	viewer_frame->resize (QSize (3, 2));

	auto* layout = new QHBoxLayout (viewer_frame);
	layout->addWidget (&*_rigid_body_viewer);
	layout->setMargin (0);

	return viewer_frame;
}


QWidget*
SimulatorWidget::make_simulation_controls()
{
	auto const ph = PaintHelper (*this);

	auto* start_stop_sim_button = new QPushButton ("Start/stop simulation", this);
	auto const update_start_stop_icon = [this, start_stop_sim_button] {
		if (_rigid_body_viewer)
		{
			auto const& icon = _rigid_body_viewer->playback() == RigidBodyViewer::Playback::Running
				? _pause_icon
				: _start_icon;

			start_stop_sim_button->setIcon (icon);
		}
	};
	QObject::connect (start_stop_sim_button, &QPushButton::pressed, [this, update_start_stop_icon] {
		if (_rigid_body_viewer)
			_rigid_body_viewer->toggle_pause();

		update_start_stop_icon();
	});
	update_start_stop_icon();

	auto* step_sim_button = new QPushButton ("Single step", this);
	QObject::connect (step_sim_button, &QPushButton::pressed, [this, update_start_stop_icon] {
		if (_rigid_body_viewer)
			_rigid_body_viewer->step();

		update_start_stop_icon();
	});

	auto* show_configurator_button = new QPushButton ("Show machine config", this);
	QObject::connect (show_configurator_button, &QPushButton::clicked, [this] {
		if (_machine)
			_machine->show_configurator();
	});

	auto* sim_controls = new QWidget (this);
	sim_controls->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	auto time_step_text = std::format ("Δt = {} s", _simulator.time_step().in<si::Second>());
	auto* time_step_label = new QLabel (QString::fromStdString (time_step_text), this);
	time_step_label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	_simulation_time_label.emplace ("", this);
	time_step_label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	update_simulation_time_label();

	auto* layout = new QHBoxLayout (sim_controls);
	layout->setMargin (0);
	layout->addWidget (start_stop_sim_button);
	layout->addWidget (step_sim_button);
	layout->addWidget (show_configurator_button);
	layout->addItem (new QSpacerItem (ph.em_pixels_int (1.0), 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
	layout->addWidget (time_step_label);
	layout->addItem (new QSpacerItem (ph.em_pixels_int (1.0), 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
	layout->addWidget (&*_simulation_time_label);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	return sim_controls;
}


QWidget*
SimulatorWidget::make_body_controls()
{
	_body_editor.emplace (this);
	_bodies_tree.emplace (this, _simulator.rigid_body_system(), *_rigid_body_viewer);

	QObject::connect (&*_bodies_tree, &QTreeWidget::currentItemChanged, [this] (QTreeWidgetItem* current, [[maybe_unused]] QTreeWidgetItem* previous) {
		if (_body_editor)
		{
			if (auto* body_item = dynamic_cast<BodyItem*> (current))
				_body_editor->edit_body (&body_item->body());
			else
				_body_editor->edit_body (nullptr);
		}
	});

	QObject::connect (&*_bodies_tree, &QTreeWidget::itemChanged, [this] (QTreeWidgetItem* item, int column) {
		if (column == 0)
		{
			if (auto* body_item = dynamic_cast<BodyItem*> (item))
			{
				body_item->backpropagate();

				if (_body_editor)
					_body_editor->refresh();
			}
			else if (auto* constraint_item = dynamic_cast<ConstraintItem*> (item))
				constraint_item->backpropagate();
		}
	});

	auto* body_controls = new QWidget (this);
	body_controls->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Expanding);

	auto* layout = new QHBoxLayout (body_controls);
	layout->setMargin (0);
	layout->addWidget (&*_bodies_tree);
	layout->addWidget (&*_body_editor);

	return body_controls;
}


void
SimulatorWidget::update_simulation_time_label()
{
	auto text = std::format ("Simulation time: {:.6f} s", _simulator.simulation_time().in<si::Second>());
	_simulation_time_label->setText (QString::fromStdString (text));
}

} // namespace xf

