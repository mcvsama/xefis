/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
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
#include "processing_loop_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/module_configurator/config_widget.h>
#include <xefis/support/ui/paint_helper.h>

// Neutrino:
#include <neutrino/math/histogram.h>
#include <neutrino/numeric.h>

// Qt:
#include <QBoxLayout>
#include <QGridLayout>
#include <QTabWidget>

// Standard:
#include <cstddef>


namespace xf::configurator {

ProcessingLoopWidget::ProcessingLoopWidget (ProcessingLoop& processing_loop, QWidget* parent):
	ConfigWidget (parent),
	_processing_loop (processing_loop)
{
	auto const ph = PaintHelper (*this, palette(), font());
	auto* name_label = create_colored_strip_label (QString::fromStdString (_processing_loop.instance()).toHtmlEscaped(), QColor (0xff, 0xd7, 0), Qt::AlignBottom, this);

	auto tabs = new QTabWidget (this);
	tabs->addTab (create_performance_tab(), "Performance");

	auto* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (name_label);
	layout->addItem (new QSpacerItem (0, ph.em_pixels (0.15f), QSizePolicy::Fixed, QSizePolicy::Fixed));
	layout->addWidget (tabs);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setSingleShot (false);
	_refresh_timer->setInterval (1000_Hz / ConfigWidget::kDataRefreshRate);
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &ProcessingLoopWidget::refresh);
	_refresh_timer->start();

	refresh();
}


void
ProcessingLoopWidget::refresh()
{
	using Milliseconds = si::Quantity<si::Millisecond>;

	{
		auto const& samples = _processing_loop.communication_times();

		if (!samples.empty())
		{
			auto const [range, grid_lines] = get_max_for_axis<Milliseconds> (*std::max_element (samples.begin(), samples.end()));
			xf::Histogram<Milliseconds> histogram (samples.begin(), samples.end(), range / 100, 0.0_ms, range);

			_communication_time_histogram->set_data (histogram, { _processing_loop.period() });
			_communication_time_histogram->set_grid_lines (grid_lines);
			_communication_time_stats->set_data (histogram, std::make_optional<Milliseconds> (_processing_loop.period()));
		}
	}

	{
		auto const& samples = _processing_loop.processing_times();

		if (!samples.empty())
		{
			auto const [range, grid_lines] = get_max_for_axis<Milliseconds> (*std::max_element (samples.begin(), samples.end()));
			xf::Histogram<Milliseconds> histogram (samples.begin(), samples.end(), range / 100, 0.0_ms, range);

			_processing_time_histogram->set_data (histogram, { _processing_loop.period() });
			_processing_time_histogram->set_grid_lines (grid_lines);
			_processing_time_stats->set_data (histogram, std::make_optional<Milliseconds> (_processing_loop.period()));
		}
	}

	{
		auto const& samples = _processing_loop.processing_latencies();

		if (!samples.empty())
		{
			auto const minmax = std::minmax_element (samples.begin(), samples.end());
			auto const min = *minmax.first;
			auto const max = *minmax.second;
			auto const [range, grid_lines] = get_max_for_axis<Milliseconds> (std::max (-min, max));
			xf::Histogram<Milliseconds> histogram (samples.begin(), samples.end(), range / 50, -range, range);

			_processing_latency_histogram->set_data (histogram, { -_processing_loop.period(), _processing_loop.period() });
			_processing_latency_histogram->set_grid_lines (2 * grid_lines);
			_processing_latency_stats->set_data (histogram);
		}
	}
}


QWidget*
ProcessingLoopWidget::create_performance_tab()
{
	auto* widget = new QWidget (this);
	QWidget* communication_time_group {};
	QWidget* processing_time_group {};
	QWidget* processing_latency_group {};

	std::tie (_communication_time_histogram, _communication_time_stats, communication_time_group) = create_performance_widget (widget, "HW communication time");
	std::tie (_processing_time_histogram, _processing_time_stats, processing_time_group) = create_performance_widget (widget, "Processing time");
	std::tie (_processing_latency_histogram, _processing_latency_stats, processing_latency_group) = create_performance_widget (widget, "Processing latency");

	auto layout = new QGridLayout (widget);
	layout->setMargin (0);
	layout->addWidget (communication_time_group, 0, 0);
	layout->addWidget (processing_time_group, 1, 0);
	layout->addWidget (processing_latency_group, 2, 0);

	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 3, 0);

	return widget;
}

} // namespace xf::configurator

