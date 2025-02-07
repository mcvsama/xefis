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
#include "items_tree.h"
#include "body_item.h"
#include "simulator_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/support/simulation/simulator.h>
#include <xefis/support/ui/paint_helper.h>

// Neutrino:
#include <neutrino/qt/qstring.h>

// Qt:
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QSlider>
#include <QShortcut>
#include <QSplitter>

// Standard:
#include <cstddef>
#include <chrono>


namespace xf {

SimulatorWidget::SimulatorWidget (Simulator& simulator, QWidget* parent):
	QWidget (parent),
	_simulator (simulator)
{
	setWindowTitle ("Xefis simulator");
	auto const ph = PaintHelper (*this);

	auto* splitter = new QSplitter (this);
	splitter->addWidget (make_viewer_widget());
	splitter->addWidget (make_body_controls());
	splitter->setHandleWidth (ph.em_pixels (0.5));
	splitter->setStretchFactor (0, 4);
	splitter->setStretchFactor (1, 2);
	splitter->setSizes ({ ph.em_pixels_int (30), ph.em_pixels_int (40) });

	auto* layout = new QVBoxLayout (this);
	layout->addWidget (make_simulation_controls());
	layout->addWidget (splitter);

	_items_tree->refresh();

	if (auto* item = _items_tree->topLevelItem (0))
		update_editor_for (item);

	resize (QSize (ph.em_pixels (80.0), ph.em_pixels (40.0)));
}


QWidget*
SimulatorWidget::make_viewer_widget()
{
	_rigid_body_viewer.emplace (this, RigidBodyViewer::AutoFPS);
	_rigid_body_viewer->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	_rigid_body_viewer->set_rigid_body_system (&_simulator.rigid_body_system());
	_rigid_body_viewer->set_redraw_callback ([this] (std::optional<si::Time> const frame_duration) {
		if (frame_duration)
			_simulator.evolve (*frame_duration * _simulation_speed);
		else
			_simulator.evolve (1);

		update_simulation_time_label();
		update_simulation_performance_label (frame_duration.value_or (0_ms));
		_group_editor->refresh();
		_body_editor->refresh();
		_constraint_editor->refresh();
	});

	auto* viewer_frame = new QFrame (this);
	viewer_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	viewer_frame->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	// Related to splitter's stretch factors:
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

	auto* step_sim_button = new QPushButton (QString::fromStdString (std::format ("Single step: Δt = {} s", _simulator.frame_duration().in<si::Second>())), this);
	QObject::connect (step_sim_button, &QPushButton::pressed, [this, update_start_stop_icon] {
		if (_rigid_body_viewer)
			_rigid_body_viewer->step();

		update_start_stop_icon();
	});

	auto* speed_label = new QLabel ("-");
	speed_label->setFixedWidth (ph.em_pixels_int (4));

	auto* speed_slider = new QSlider (Qt::Horizontal);
	speed_slider->setTickPosition (QSlider::TicksAbove);
	speed_slider->setTracking (true);
	speed_slider->setTickInterval (10);
	speed_slider->setPageStep (10);
	speed_slider->setRange (1, 200);
	QObject::connect (speed_slider, &QSlider::valueChanged, [this, speed_label] (int value) {
		_simulation_speed = value / 100.0f;
		speed_label->setText (to_qstring (std::format (" {:d}%", value)));
	});
	speed_slider->setValue (100);

	auto* show_configurator_button = new QPushButton ("Show machine config", this);
	QObject::connect (show_configurator_button, &QPushButton::clicked, [this] {
		if (_machine)
			_machine->show_configurator();
	});

	auto* sim_controls = new QWidget (this);
	sim_controls->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	_simulation_time_label.emplace ("", this);
	_simulation_time_label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	update_simulation_time_label();

	_simulation_performance_value_label.emplace ("", this);
	_simulation_performance_value_label->setFixedWidth (ph.em_pixels_int (4));

	update_simulation_performance_label (0_ms);

	auto* basis_colors_label = new QLabel ("<b><span style='color: red'>X (Null Island)</span> <span style='color: green'>Y</span> <span style='color: blue'>Z (North Pole)</span></b>", this);

	auto* row1_layout = new QHBoxLayout();
	row1_layout->setMargin (0);
	row1_layout->addWidget (start_stop_sim_button);
	row1_layout->addWidget (step_sim_button);
	row1_layout->addItem (new QSpacerItem (ph.em_pixels_int (1.0), 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
	row1_layout->addWidget (basis_colors_label);
	row1_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	row1_layout->addWidget (show_configurator_button);

	auto* row2_layout = new QHBoxLayout();
	row2_layout->setMargin (0);
	row2_layout->addWidget (new QLabel ("Speed: "));
	row2_layout->addWidget (speed_slider);
	row2_layout->addWidget (speed_label);
	row2_layout->addItem (new QSpacerItem (ph.em_pixels_int (1.0), 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
	row2_layout->addWidget (new QLabel ("Performance: "));
	row2_layout->addWidget (&*_simulation_performance_value_label);
	row2_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	row2_layout->addWidget (&*_simulation_time_label);

	auto* sim_controls_layout = new QVBoxLayout (sim_controls);
	sim_controls_layout->setMargin (0);
	sim_controls_layout->addLayout (row1_layout);
	sim_controls_layout->addLayout (row2_layout);

	return sim_controls;
}


QWidget*
SimulatorWidget::make_body_controls()
{
	_group_editor.emplace (this, *_rigid_body_viewer, Qt::blue);
	_body_editor.emplace (this, *_rigid_body_viewer, Qt::darkGreen);
	_constraint_editor.emplace (this, *_rigid_body_viewer, QColor (0xff, 0x8c, 0));
	_items_tree.emplace (this, _simulator.rigid_body_system(), *_rigid_body_viewer);
	_items_tree->setMouseTracking (true);

	_editors_stack.emplace (this);
	_editors_stack->addWidget (&*_group_editor);
	_editors_stack->addWidget (&*_body_editor);
	_editors_stack->addWidget (&*_constraint_editor);

	QObject::connect (&*_items_tree, &QTreeWidget::currentItemChanged, [this] (QTreeWidgetItem* current, [[maybe_unused]] QTreeWidgetItem* previous) {
		update_editor_for (current);
	});

	QObject::connect (&*_items_tree, &QTreeWidget::itemEntered, [this] (QTreeWidgetItem* current, [[maybe_unused]] int column) {
		if (!current)
			_rigid_body_viewer->set_hovered (nullptr);
		else if (auto* body_item = dynamic_cast<BodyItem*> (current))
			_rigid_body_viewer->set_hovered (&body_item->body());
		else
			_rigid_body_viewer->set_hovered (nullptr);
	});

	QObject::connect (&*_items_tree, &QTreeWidget::itemChanged, [this] (QTreeWidgetItem* item, int column) {
		if (column == 0)
		{
			if (auto* group_item = dynamic_cast<GroupItem*> (item))
			{
				group_item->backpropagate();
				_items_tree->refresh();

				if (_group_editor)
					_group_editor->refresh();
			}
			else if (auto* body_item = dynamic_cast<BodyItem*> (item))
			{
				body_item->backpropagate();
				_items_tree->refresh();

				if (_body_editor)
					_body_editor->refresh();
			}
			else if (auto* constraint_item = dynamic_cast<ConstraintItem*> (item))
			{
				constraint_item->backpropagate();
				_items_tree->refresh();

				if (_constraint_editor)
					_constraint_editor->refresh();
			}
		}
	});

	auto* body_controls = new QWidget (this);
	body_controls->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Expanding);

	auto* layout = new QHBoxLayout (body_controls);
	layout->setMargin (0);
	layout->addWidget (&*_items_tree);
	layout->addWidget (&*_editors_stack);

	return body_controls;
}


void
SimulatorWidget::update_editor_for (QTreeWidgetItem* item)
{
	if (auto* group_item = dynamic_cast<GroupItem*> (item))
	{
		auto& group = group_item->group();
		_group_editor->edit (&group);
		_editors_stack->setCurrentWidget (&*_group_editor);
		_rigid_body_viewer->set_focused (group);
	}
	else if (auto* body_item = dynamic_cast<BodyItem*> (item))
	{
		auto& body = body_item->body();
		_body_editor->edit (&body);
		_editors_stack->setCurrentWidget (&*_body_editor);
		_rigid_body_viewer->set_focused (body);
	}
	else if (auto* constraint_item = dynamic_cast<ConstraintItem*> (item))
	{
		_constraint_editor->edit (&constraint_item->constraint());
		_editors_stack->setCurrentWidget (&*_constraint_editor);
	}
	else
	{
		_group_editor->edit (nullptr);
		_body_editor->edit (nullptr);
		_constraint_editor->edit (nullptr);
	}
}


void
SimulatorWidget::update_simulation_time_label()
{
	auto const simulation_time = std::chrono::system_clock::from_time_t (_simulator.simulation_time().in<si::Second>());
	auto const elapsed_time = _simulator.elapsed_time().in<si::Second>();
	auto const text = std::format ("{:%Y-%m-%d %H:%M:%S} UTC ({:.6f} s)", simulation_time, elapsed_time);
	_simulation_time_label->setText (QString::fromStdString (text));
}


void
SimulatorWidget::update_simulation_performance_label (si::Time const dt)
{
	auto perf = _simulator.performance();

	if (std::isfinite (perf))
		_last_finite_performance = perf;
	else
		perf = _last_finite_performance;

	perf = _performance_smoother (perf, dt);
	auto const text = std::format ("{:.0f}%", 100.0f * perf);
	auto const prefix = perf < 1.0 ? "<span style='color: red'>" : "<span>";
	auto const suffix = "</span>";
	_simulation_performance_value_label->setText (prefix + QString::fromStdString (text) + suffix);
}

} // namespace xf

