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
#include "debug.h"
#include "debug_performance.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/paint_helper.h>
#include <xefis/support/ui/performance_widget.h>

// Neutrino:
#include <neutrino/synchronized.h>

// Qt:
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QMetaObject>
#include <QPainter>
#include <QPushButton>
#include <QThread>

// Lib:
#include <boost/circular_buffer.hpp>

// Standard:
#include <algorithm>
#include <cstddef>
#include <map>
#include <optional>
#include <vector>


namespace xf {
namespace {

using TimeHistogram = math::Histogram<si::Time>;

struct DebugPerformanceMeasurement
{
	HistogramWidget*					histogram_widget	{ nullptr };
	HistogramStatsWidget*				stats_widget		{ nullptr };
	boost::circular_buffer<si::Time>	samples				{ 1u };
	DebugPerformanceParameters			parameters;
	bool								needs_ui_update		{ false };
};


struct DebugPerformanceSnapshot
{
	HistogramWidget*			histogram_widget;
	HistogramStatsWidget*		stats_widget;
	std::vector<si::Time>		samples;
	TimeHistogram::Parameters	histogram_parameters;
};


struct DebugPerformanceState
{
	std::map<std::string, DebugPerformanceMeasurement>	measurements;
	bool												ui_update_queued	{ false };
};


[[nodiscard]]
auto
debug_performance_state()
{
	static nu::Synchronized<DebugPerformanceState> state;
	return state.lock();
}


[[nodiscard]]
QString
sanitize_filename_component (QString component)
{
	component = component.trimmed();
	component.replace ("/", "_");

	if (component.isEmpty())
		return "measurement";
	else
		return component;
}


[[nodiscard]]
QString
default_screenshot_filename (QString const& measurement_name)
{
	auto const timestamp = QDateTime::currentDateTime().toString (Qt::ISODate);
	return QDir::homePath() + "/" + timestamp + " " + sanitize_filename_component (measurement_name) + ".png";
}


[[nodiscard]]
QString
choose_screenshot_filename (QWidget* parent, QString const& measurement_name)
{
	auto filename = QFileDialog::getSaveFileName (
		parent,
		"Save histogram screenshot",
		default_screenshot_filename (measurement_name),
		"PNG images (*.png)"
	);

	if (filename.isEmpty())
		return {};

	if (!filename.endsWith (".png", Qt::CaseInsensitive))
		filename += ".png";

	return filename;
}


[[nodiscard]]
QPixmap
make_histogram_screenshot (HistogramWidget* histogram_widget, HistogramStatsWidget* stats_widget)
{
	auto const histogram_pixmap = histogram_widget->grab();
	auto const stats_pixmap = stats_widget->grab();
	auto const width = std::max (histogram_pixmap.width(), stats_pixmap.width());
	auto const height = histogram_pixmap.height() + stats_pixmap.height();

	auto screenshot = QPixmap (width, height);
	screenshot.fill (Qt::white);

	auto painter = QPainter (&screenshot);
	painter.drawPixmap (0, 0, histogram_pixmap);
	painter.drawPixmap (0, histogram_pixmap.height(), stats_pixmap);

	return screenshot;
}


void
save_histogram_screenshot (HistogramWidget* histogram_widget, HistogramStatsWidget* stats_widget, QString const& measurement_name)
{
	if (histogram_widget && stats_widget)
	{
		if (auto const filename = choose_screenshot_filename (stats_widget, measurement_name);
			!filename.isEmpty())
		{
			make_histogram_screenshot (histogram_widget, stats_widget).save (filename, "PNG");
		}
	}
}


void
configure_sample_storage (DebugPerformanceMeasurement& measurement)
{
	auto const capacity = std::max (std::size_t (1), measurement.parameters.max_samples);

	if (measurement.samples.capacity() != capacity)
	{
		auto new_samples = boost::circular_buffer<si::Time> (capacity);
		auto const start = measurement.samples.size() > capacity
			? measurement.samples.size() - capacity
			: 0u;

		for (std::size_t i = start; i < measurement.samples.size(); ++i)
			new_samples.push_back (measurement.samples[i]);

		measurement.samples = std::move (new_samples);
	}
}


void
update_debug_performance_widgets()
{
	if (auto* app = QApplication::instance())
	{
		// The UI stuff happens on main thread, but the measurements might be on non-main thread.
		// Have to request the UI update from the main thread like this:
		if (QThread::currentThread() != app->thread())
		{
			auto state = debug_performance_state();

			if (!std::exchange (state->ui_update_queued, true))
			{
				state.unlock();
				QMetaObject::invokeMethod (app, []{ update_debug_performance_widgets(); }, Qt::QueuedConnection);
			}
		}
		else
		{
			std::vector<DebugPerformanceSnapshot> snapshots;

			{
				auto state = debug_performance_state();
				state->ui_update_queued = false;

				auto& window_layout = get_debug_window_layout();

				for (auto& [name, measurement]: state->measurements)
				{
					if (!measurement.stats_widget)
					{
						QWidget* performance_group = nullptr;
						std::tie (measurement.histogram_widget, measurement.stats_widget, performance_group)
							= create_performance_widget (nullptr, nu::to_qstring (name));
						measurement.histogram_widget->set_y_legend_visible (true);
						auto* screenshot_button = new QPushButton ("Save histogram screenshot", nullptr);
						auto const measurement_name = nu::to_qstring (name);

						QObject::connect (
							screenshot_button,
							&QPushButton::clicked,
							[histogram_widget = measurement.histogram_widget, stats_widget = measurement.stats_widget, measurement_name] {
								save_histogram_screenshot (histogram_widget, stats_widget, measurement_name);
							}
						);

						window_layout.addWidget (PaintHelper::new_hline());
						window_layout.addWidget (performance_group);
						window_layout.addWidget (screenshot_button);
					}

					if (measurement.needs_ui_update && !measurement.samples.empty())
					{
						snapshots.push_back (DebugPerformanceSnapshot {
							measurement.histogram_widget,
							measurement.stats_widget,
							std::vector<si::Time> (measurement.samples.begin(), measurement.samples.end()),
							TimeHistogram::Parameters {
								.bin_width = measurement.parameters.bin_width,
								.min_x = measurement.parameters.min_x,
								.max_x = measurement.parameters.max_x,
							},
						});

						measurement.needs_ui_update = false;
					}
				}
			}

			for (auto const& snapshot: snapshots)
			{
				auto const histogram = TimeHistogram (snapshot.samples.begin(), snapshot.samples.end(), snapshot.histogram_parameters);
				snapshot.histogram_widget->set_data (histogram);
				snapshot.stats_widget->set_data (histogram);
			}
		}
	}
}


void
ensure_debug_performance_measurement_exists (std::string const& name, DebugPerformanceParameters const& parameters)
{
	{
		auto state = debug_performance_state();
		auto [it, inserted] = state->measurements.try_emplace (name);
		auto& measurement = it->second;

		auto const config_changed = inserted ||
									measurement.parameters.bin_width != parameters.bin_width ||
									measurement.parameters.min_x != parameters.min_x ||
									measurement.parameters.max_x != parameters.max_x ||
									measurement.parameters.max_samples != parameters.max_samples ||
									measurement.parameters.stop_when_full != parameters.stop_when_full;

		measurement.parameters = parameters;
		configure_sample_storage (measurement);

		if (config_changed && !measurement.samples.empty())
			measurement.needs_ui_update = true;
	}

	update_debug_performance_widgets();
}


void
store_performance_measurement_sample (std::string const& name, si::Time const duration)
{
	{
		auto state = debug_performance_state();
		auto const found = state->measurements.find (name);

		if (found == state->measurements.end())
			return;

		auto& measurement = found->second;

		if (measurement.parameters.stop_when_full && measurement.samples.full())
			return;

		measurement.samples.push_back (duration);
		measurement.needs_ui_update = true;
	}

	update_debug_performance_widgets();
}

} // namespace


nu::ScopeExit<>
debug_measure_performance (std::string const& name, DebugPerformanceParameters parameters)
{
	if (parameters.max_samples == 0u)
		parameters.max_samples = 1u;

	ensure_debug_performance_measurement_exists (name, parameters);

	auto const t0 = nu::steady_now();

	// ScopeExit allows both explicit stop() and implicit stop-on-destruction.
	// In either case, measured duration is appended to the persistent per-name sample set.
	return nu::ScopeExit<> (std::function<void()> ([name, t0] {
		store_performance_measurement_sample (name, nu::steady_now() - t0);
	}));
}

} // namespace xf
