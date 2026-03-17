/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
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
#include "iterative_solver_stats_widget.h"

// Xefis:
#include <xefis/support/ui/performance_widget.h>
#include <xefis/support/ui/paint_helper.h>

// Neutrino:
#include <neutrino/math/histogram.h>
#include <neutrino/numeric.h>
#include <neutrino/qt/qstring.h>

// Qt:
#include <QLabel>
#include <QLayout>
#include <QTimer>

// Standard:
#include <algorithm>
#include <cstddef>
#include <optional>


namespace xf {

[[nodiscard]]
static std::string
format_iterations (double const iterations)
{
	return std::format ("{:.1f}", iterations);
}


IterativeSolverStatsWidget::IterativeSolverStatsWidget (SnapshotProvider snapshot_provider, QWidget* parent):
	QWidget (parent),
	_snapshot_provider (std::move (snapshot_provider))
{
	setAttribute (Qt::WA_DeleteOnClose);
	setWindowTitle ("Rigid-body impulse solver details");

	auto const ph = PaintHelper (*this);

	_note_label = new QLabel (this);
	_note_label->setWordWrap (true);

	std::tie (_iterations_histogram, _iterations_stats, _iterations_group) = create_performance_widget (this, "Iterations to convergence");
	_iterations_histogram->set_y_legend_visible (true);

	std::tie (_solve_time_histogram, _solve_time_stats, _solve_time_group) = create_performance_widget (this, "Solver wall time per frame");
	_solve_time_histogram->set_y_legend_visible (true);

	auto* layout = new QVBoxLayout (this);
	layout->setContentsMargins (ph.widget_margins());
	layout->addWidget (_note_label);
	layout->addWidget (_iterations_group);
	layout->addWidget (_solve_time_group);

	_refresh_timer = std::make_unique<QTimer> (this);
	_refresh_timer->setSingleShot (false);
	_refresh_timer->setInterval (250);
	QObject::connect (_refresh_timer.get(), &QTimer::timeout, this, &IterativeSolverStatsWidget::refresh);
	_refresh_timer->start();

	setFixedSize (sizeHint());
	refresh();
}


void
IterativeSolverStatsWidget::refresh()
{
	if (_snapshot_provider)
	{
		auto const snapshot = _snapshot_provider();

		_solve_time_marks.resize (1);
		_solve_time_marks[0] = snapshot.frame_duration;

		_note_label->setText (nu::to_qstring (std::format ("Solver iteration limit: {}", snapshot.max_iterations)));

		if (snapshot.max_iterations == 0 || snapshot.iterations_run.begin() == snapshot.iterations_run.end())
			_iterations_group->setEnabled (false);
		else
		{
			auto const max_iterations = static_cast<double> (snapshot.max_iterations);
			auto const [max_x, grid_lines] = nu::get_max_for_axis (max_iterations);
			auto const histogram = nu::math::Histogram<double> (snapshot.iterations_run.begin(), snapshot.iterations_run.end(), {
				.bin_width = std::max (1.0, max_x / 100.0),
				.min_x = 0.0,
				.max_x = max_x,
			});

			_iterations_histogram->set_data (histogram, format_iterations);
			_iterations_histogram->set_grid_lines (grid_lines);
			_iterations_stats->set_data (histogram, format_iterations);
			_iterations_group->setEnabled (true);
		}

		if (snapshot.real_time_taken.begin() == snapshot.real_time_taken.end())
			_solve_time_group->setEnabled (false);
		else
		{
			auto const [max_x, grid_lines] = nu::get_max_for_axis<si::Time> (*std::max_element (snapshot.real_time_taken.begin(), snapshot.real_time_taken.end()));
			auto const histogram = nu::math::Histogram<si::Time> (snapshot.real_time_taken.begin(), snapshot.real_time_taken.end(), {
				.bin_width = max_x / 100,
				.min_x = 0.0_ms,
				.max_x = max_x,
			});

			_solve_time_histogram->set_data (histogram, _solve_time_marks);
			_solve_time_histogram->set_grid_lines (grid_lines);
			_solve_time_stats->set_data (histogram);
			_solve_time_group->setEnabled (true);
		}
	}
	else
	{
		_note_label->setText ("Solver iteration limit: unknown");
		_iterations_group->setEnabled (false);
		_solve_time_group->setEnabled (false);
	}
}

} // namespace xf
