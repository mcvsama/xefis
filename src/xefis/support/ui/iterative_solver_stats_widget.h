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

#ifndef XEFIS__SUPPORT__UI__ITERATIVE_SOLVER_STATS_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__UI__ITERATIVE_SOLVER_STATS_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/simulator.h>

// Qt:
#include <QLabel>
#include <QTimer>
#include <QWidget>

// Standard:
#include <functional>
#include <optional>


namespace xf {

class HistogramStatsWidget;
class HistogramWidget;


class IterativeSolverStatsWidget: public QWidget
{
  public:
	struct Snapshot
	{
		Simulator::IterationsRunHistoryRange	iterations_run;
		Simulator::RealTimeTakenHistoryRange	real_time_taken;
		std::size_t								max_iterations	{ 0 };
		si::Time								frame_duration;
	};

	using SnapshotProvider = std::function<Snapshot()>;

  public:
	// Ctor
	explicit
	IterativeSolverStatsWidget (SnapshotProvider, QWidget* parent = nullptr);

  private:
	void
	refresh();

  private:
	SnapshotProvider		_snapshot_provider;

	QWidget*				_iterations_group		{ nullptr };
	HistogramWidget*		_iterations_histogram	{ nullptr };
	HistogramStatsWidget*	_iterations_stats		{ nullptr };

	QWidget*				_solve_time_group		{ nullptr };
	HistogramWidget*		_solve_time_histogram	{ nullptr };
	HistogramStatsWidget*	_solve_time_stats		{ nullptr };
	std::vector<si::Time>	_solve_time_marks;

	QLabel*					_note_label				{ nullptr };
	std::unique_ptr<QTimer>	_refresh_timer;
};

} // namespace xf

#endif
