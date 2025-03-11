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
#include "camera_controls.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/support/simulation/simulator.h>
#include <xefis/support/ui/paint_helper.h>

// Neutrino:
#include <neutrino/qt/qstring.h>

// Qt:
#include <QDateTimeEdit>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSlider>
#include <QShortcut>
#include <QSplitter>
#include <QTabWidget>

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
	_rigid_body_viewer->use_work_performer (&_graphics_work_performer);
	_rigid_body_viewer->set_rigid_body_system (&_simulator.rigid_body_system());
	_rigid_body_viewer->set_redraw_callback ([this, prev_sim_time = 0_s] (std::optional<si::Time> const frame_duration) mutable {
		if (frame_duration)
			_simulator.evolve (*frame_duration * _simulation_speed);
		else
			_simulator.evolve (1);

		update_simulation_time_label();
		update_simulation_performance_label (frame_duration.value_or (0_ms));

		update_viewer_time();

		// Avoid calling update_solar_time_widget() too often as it causes Qt signals and Qt's signals are extremely slow.
		if (_simulator.simulation_time() - prev_sim_time > 1_s)
		{
			update_solar_time_widgets();
			prev_sim_time = 1_s * std::floor (_simulator.simulation_time().in<si::Second>());
		}

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

	auto* speed_label = new QLabel ("–");
	speed_label->setFixedWidth (ph.em_pixels_int (4));

	auto* speed_slider = new QSlider (Qt::Horizontal);
	speed_slider->setTickPosition (QSlider::TicksAbove);
	speed_slider->setTracking (true);
	speed_slider->setTickInterval (10);
	speed_slider->setPageStep (10);
	speed_slider->setRange (1, 200);
	QObject::connect (speed_slider, &QSlider::valueChanged, [this, speed_label] (int value) {
		_simulation_speed = value / 100.0f;
		speed_label->setText (to_qstring (std::format ("{:d}%", value)));
	});
	speed_slider->setValue (100);

	auto* tabs = new QTabWidget (this);
	tabs->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	tabs->addTab (make_solar_time_controls (ph), "Solar time");
	tabs->addTab (new CameraControls(), "Camera");

	auto* sim_controls = new QWidget (this);
	sim_controls->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

	_simulation_time_label.emplace ("", this);
	update_simulation_time_label();

	_simulation_performance_value_label.emplace ("–", this);
	_simulation_performance_value_label->setFixedWidth (ph.em_pixels_int (4));

	update_simulation_performance_label (0_ms);

	auto* basic_controls = new QWidget (this);
	basic_controls->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Fixed);
	basic_controls->setMinimumWidth (ph.em_pixels_int (25));

	// Layout:
	{
		auto* basic_controls_layout = new QGridLayout (basic_controls);
		{
			basic_controls_layout->setMargin (0);
			auto row = 0;
			auto* buttons_layout = new QHBoxLayout();
			buttons_layout->addWidget (start_stop_sim_button);
			buttons_layout->addWidget (step_sim_button);
			basic_controls_layout->addLayout (buttons_layout, row, 0, 1, 3);
			++row;
			basic_controls_layout->addWidget (new QLabel ("Speed: "), row, 0);
			speed_label->setAlignment (Qt::AlignRight);
			basic_controls_layout->addWidget (speed_label, row, 1);
			basic_controls_layout->addWidget (speed_slider, row, 2);
			++row;
			basic_controls_layout->addWidget (new QLabel ("Performance: "), row, 0);
			_simulation_performance_value_label->setAlignment (Qt::AlignRight);
			basic_controls_layout->addWidget (&*_simulation_performance_value_label, row, 1);
			++row;
			basic_controls_layout->addWidget (new QLabel ("Time: "), row, 0, 1, 1);
			basic_controls_layout->addWidget (&*_simulation_time_label, row, 1, 1, 2);
			++row;
			basic_controls_layout->addWidget (ph.new_hline(), row, 0, 1, 3);
			++row;
			auto* basis_colors_label = new QLabel ("<b><span style='color: red'>X (Null Island)</span> <span style='color: green'>Y (90°E, 0°N)</span> <span style='color: blue'>Z (North Pole)</span></b>", this);
			basic_controls_layout->addWidget (basis_colors_label, row, 0, 1, 3);
		}

		auto* sim_controls_layout = new QHBoxLayout (sim_controls);
		sim_controls_layout->setMargin (0);
		sim_controls_layout->addWidget (basic_controls);
		sim_controls_layout->addWidget (tabs);
		sim_controls_layout->setStretch (0, 0);
		sim_controls_layout->setStretch (1, 1);

		// setTabOrder() must be after setting up the layout:
		QWidget::setTabOrder (step_sim_button, speed_slider);
	}

	return sim_controls;
}


QWidget*
SimulatorWidget::make_solar_time_controls (PaintHelper const& ph)
{
	auto* time_widget = new QWidget (this);

	_day_of_year_slider.emplace (Qt::Horizontal);
	_day_of_year_slider->setTickPosition (QSlider::TicksAbove);
	_day_of_year_slider->setTracking (true);
	_day_of_year_slider->setTickInterval (30);
	_day_of_year_slider->setPageStep (30);
	_day_of_year_slider->setRange (0, 364);
	_day_of_year_slider->setMinimumWidth (ph.em_pixels_int (8.0));
	QObject::connect (&*_day_of_year_slider, &QSlider::valueChanged, [this, locked = false] (int day_of_year) mutable {
		if (auto const lock = bool_lock (locked))
		{
			if (_solar_date_time_edit)
			{
				auto date_time = _solar_date_time_edit->dateTime().toUTC();
				auto date = date_time.date();
				date.setDate (date.year(), 1, 1);
				date = date.addDays (day_of_year);
				date_time.setDate (date);
				set_solar_time (date_time);
			}
		}
	});

	_time_of_day_slider.emplace (Qt::Horizontal);
	_time_of_day_slider->setTickPosition (QSlider::TicksAbove);
	_time_of_day_slider->setTracking (true);
	_time_of_day_slider->setTickInterval (60);
	_time_of_day_slider->setPageStep (60);
	_time_of_day_slider->setRange (0, 60 * 24);
	_time_of_day_slider->setMinimumWidth (ph.em_pixels_int (8.0));
	QObject::connect (&*_time_of_day_slider, &QSlider::valueChanged, [this, locked = false] (int minute_of_the_day) mutable {
		if (auto const lock = bool_lock (locked))
		{
			if (_solar_date_time_edit)
			{
				auto date_time = _solar_date_time_edit->dateTime().toUTC();
				auto time = _solar_date_time_edit->time();
				time.setHMS (minute_of_the_day / 60, minute_of_the_day % 60, 0);
				date_time.setTime (time);
				set_solar_time (date_time);
			}
		}
	});

	_solar_date_time_edit.emplace();
	_solar_date_time_edit->setTimeSpec (Qt::UTC);
	QObject::connect (&*_solar_date_time_edit, &QDateTimeEdit::dateTimeChanged, [this, locked = false] (QDateTime const& date_time) mutable {
		if (auto const lock = bool_lock (locked))
			set_solar_time (date_time);
	});

	auto* set_to_simulation_time = new QPushButton ("Set to simulation time");
	QObject::connect (set_to_simulation_time, &QPushButton::clicked, [this] {
		_solar_simulation_time_delta = 0_s;
		update_solar_time_widgets();
	});

	auto* set_to_system_time = new QPushButton ("Set to system time");
	QObject::connect (set_to_system_time, &QPushButton::clicked, [this] {
		_solar_simulation_time_delta = TimeHelper::utc_now() - _simulator.simulation_time();
		update_solar_time_widgets();
	});

	auto* set_to_local_noon = new QPushButton ("Set to local noon");
	auto set_to_local_noon_callback = [this] {
		auto date_time = QDateTime::fromSecsSinceEpoch (TimeHelper::utc_now().in<si::Second>());
		date_time.setTime (QTime (12, 0, 0));
		_solar_simulation_time_delta = 1_s * date_time.toUTC().toSecsSinceEpoch() - _simulator.simulation_time();
		update_solar_time_widgets();
	};
	QObject::connect (set_to_local_noon, &QPushButton::clicked, set_to_local_noon_callback);

	// Layout:
	{
		auto* buttons_layout = new QHBoxLayout();
		buttons_layout->addWidget (set_to_simulation_time);
		buttons_layout->addWidget (set_to_system_time);
		buttons_layout->addWidget (set_to_local_noon);
		buttons_layout->addItem (ph.new_expanding_horizontal_spacer());

		auto* utc_month_label = new QLabel ("UTC day of year: ");
		auto* utc_time_of_day_label = new QLabel ("UTC time of day: ");
		auto* utc_date_and_time = new QLabel ("UTC date and time: ");

		for (auto* widget: { utc_month_label, utc_time_of_day_label, utc_date_and_time })
			widget->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Fixed);

		auto* layout = new QGridLayout (time_widget);
		auto row = 0;
		layout->addWidget (utc_month_label, row, 0);
		layout->addWidget (&*_day_of_year_slider, row, 1, 1, 2);
		++row;
		layout->addWidget (utc_time_of_day_label, row, 0);
		layout->addWidget (&*_time_of_day_slider, row, 1, 1, 2);
		++row;
		layout->addWidget (utc_date_and_time, row, 0);
		layout->addWidget (&*_solar_date_time_edit, row, 1);
		layout->addLayout (buttons_layout, row, 2);
	}

	// By default set solar time to local noon:
	set_to_local_noon_callback();

	return time_widget;
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

	auto item_changed_connection = QObject::connect (&*_items_tree, &QTreeWidget::currentItemChanged, [this] (QTreeWidgetItem* current, [[maybe_unused]] QTreeWidgetItem* previous) {
		update_editor_for (current);
	});

	_disconnect_item_changed_signal.emplace ([item_changed_connection] {
		QObject::disconnect (item_changed_connection);
	});

	QObject::connect (&*_items_tree, &QTreeWidget::itemEntered, [this] (QTreeWidgetItem* current, [[maybe_unused]] int column) {
		if (!current)
			_rigid_body_viewer->set_hovered_to_none();
		else if (auto* body_item = dynamic_cast<BodyItem*> (current))
			_rigid_body_viewer->set_hovered (body_item->body());
		else if (auto* constraint_item = dynamic_cast<ConstraintItem*> (current))
			_rigid_body_viewer->set_hovered (constraint_item->constraint());
		else
			_rigid_body_viewer->set_hovered_to_none();
	});

	QObject::connect (&*_items_tree, &QTreeWidget::itemChanged, [this] (QTreeWidgetItem* item, int column) {
		if (column == 0)
		{
			auto backpropagate = [this, &item]<class SpecificItem, class Editor> (Editor& editor) -> bool {
				if (auto* specific_item = dynamic_cast<SpecificItem*> (item))
				{
					specific_item->backpropagate();
					_items_tree->refresh();

					if (editor)
						editor->refresh();

					return true;
				}
				else
					return false;
			};

			if (backpropagate.operator()<GroupItem> (_group_editor))
			{ }
			else if (backpropagate.operator()<BodyItem> (_body_editor))
			{ }
			else if (backpropagate.operator()<ConstraintItem> (_constraint_editor))
			{ }
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
		auto& constraint = constraint_item->constraint();
		_constraint_editor->edit (&constraint);
		_editors_stack->setCurrentWidget (&*_constraint_editor);
		_rigid_body_viewer->set_focused (constraint);
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
	auto const rounded_simulation_time = std::chrono::time_point_cast<std::chrono::seconds> (simulation_time);
	auto const elapsed_time = _simulator.elapsed_time().in<si::Second>();
	auto const text = std::format ("{:%Y-%m-%d %H:%M:%S} UTC ({:.6f} s)", rounded_simulation_time, elapsed_time);
	_simulation_time_label->setText (QString::fromStdString (text));
}


void
SimulatorWidget::update_viewer_time()
{
	_rigid_body_viewer->set_time (solar_time());
}


void
SimulatorWidget::set_solar_time (QDateTime const date_time)
{
	_solar_simulation_time_delta = 1_s * date_time.toUTC().toSecsSinceEpoch() - _simulator.simulation_time();
	update_solar_time_widgets();
}


void
SimulatorWidget::update_solar_time_widgets()
{
	auto const date_time = QDateTime::fromSecsSinceEpoch (solar_time().in<si::Second>(), Qt::UTC);
	auto const date = date_time.date();
	auto const time = date_time.time();

	// Update the day of the year slider:
	if (_day_of_year_slider)
	{
		auto const day_of_year_blocker = QSignalBlocker (&*_day_of_year_slider);
		_day_of_year_slider->setValue (date.dayOfYear() - 1);
	}
	// Update the time of the day slider:
	if (_time_of_day_slider)
	{
		auto const time_of_day_blocker = QSignalBlocker (&*_time_of_day_slider);
		_time_of_day_slider->setValue (time.hour() * 60 + time.minute());
	}
	// Update date-time entry widget:
	if (_solar_date_time_edit)
	{
		auto const blocker = QSignalBlocker (&*_solar_date_time_edit);
		_solar_date_time_edit->setDateTime (QDateTime::fromSecsSinceEpoch (solar_time().in<si::Second>(), Qt::UTC));
	}

	update_viewer_time();
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

