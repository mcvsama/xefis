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

// Standard:
#include <cstddef>

// Qt:
#include <QBoxLayout>
#include <QGridLayout>
#include <QTabWidget>

// Neutrino:
#include <neutrino/math/histogram.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/module_configurator/config_widget.h>

// Local:
#include "screen_widget.h"


namespace xf::configurator {

ScreenWidget::ScreenWidget (Screen& screen, QWidget* parent):
	ConfigWidget (parent),
	_screen (screen)
{
	auto* name_label = create_colored_strip_label (QString::fromStdString (_screen.instance()).toHtmlEscaped(), QColor (0xff, 0xaa, 0x00), Qt::AlignBottom, this);

	auto tabs = new QTabWidget (this);
	tabs->addTab (create_performance_tab(), "Performance");

	auto* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (name_label);
	layout->addItem (new QSpacerItem (0, em_pixels (0.15f), QSizePolicy::Fixed, QSizePolicy::Fixed));
	layout->addWidget (tabs);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setSingleShot (false);
	_refresh_timer->setInterval (1000_Hz / ConfigWidget::kDataRefreshRate);
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &ScreenWidget::refresh);
	_refresh_timer->start();

	refresh();
}


void
ScreenWidget::refresh()
{
	using Milliseconds = si::Quantity<si::Millisecond>;

	// TODO show total time spent on rendering instruments synchronously
	// TODO show total time spent on rendering information asynchronous

	for (auto const& widgets_pair: _work_performer_widgets)
	{
		if (auto metrics = _screen.work_performer_metrics_for (widgets_pair.first))
		{
			auto const& widgets = widgets_pair.second;

			auto const update_histogram = [](auto const& samples, auto& histogram_widget, auto& stats_widget)
			{
				if (!samples.empty())
				{
					auto const [range, grid_lines] = get_max_for_axis<Milliseconds> (*std::max_element (samples.begin(), samples.end()));
					xf::Histogram<Milliseconds> histogram (samples.begin(), samples.end(), range / 100, 0.0_ms, range);

					histogram_widget.set_data (histogram);
					histogram_widget.set_grid_lines (grid_lines);
					stats_widget.set_data (histogram);
				}
			};

			update_histogram (metrics->start_latencies, *widgets.start_latency_histogram, *widgets.start_latency_stats);
			update_histogram (metrics->total_latencies, *widgets.total_latency_histogram, *widgets.total_latency_stats);
		}
	}
}


QWidget*
ScreenWidget::create_performance_tab()
{
	auto* widget = new QWidget (this);

	// Prepare list of Widgets objects for each WorkPerformer:
	for (auto& instrument_disclosure: _screen.instrument_tracker())
	{
		auto const* work_performer = instrument_disclosure.details().work_performer;
		auto const module_name = identifier (instrument_disclosure.value());

		if (auto widgets_pair = _work_performer_widgets.find (work_performer);
			widgets_pair != _work_performer_widgets.end())
		{
			widgets_pair->second.module_names += " • " + module_name;
		}
		else
		{
			auto& widgets = _work_performer_widgets[work_performer];
			widgets.work_performer = work_performer;
			widgets.module_names = module_name;
		}
	}

	auto* tabs = new QTabWidget (widget);

	{
		unsigned int pos = 0;

		// Create widgets for each Widgets object:
		for (auto& widgets_pair: _work_performer_widgets)
		{
			auto& widgets = widgets_pair.second;

			auto* tab = new QWidget (tabs);
			tabs->addTab (tab, QString ("Work performer %1").arg (++pos));

			auto* handled_modules_info = new QLabel ("<b>Modules handled by this work performer:</b><br/>" + QString::fromStdString (widgets.module_names).toHtmlEscaped(), tab);
			handled_modules_info->setWordWrap (true);

			std::tie (widgets.start_latency_histogram, widgets.start_latency_stats, widgets.start_latency_group)
				= create_performance_widget (tab, QString::fromStdString ("Paint start latency (request start to painting start)"));

			std::tie (widgets.total_latency_histogram, widgets.total_latency_stats, widgets.total_latency_group)
				= create_performance_widget (tab, QString::fromStdString ("Total latency (request start to painting finish)"));

			auto* tab_layout = new QGridLayout (tab);
			tab_layout->setMargin (0);
			tab_layout->addWidget (widgets.start_latency_group, 0, 0);
			tab_layout->addWidget (widgets.total_latency_group, 1, 0);
			tab_layout->addItem (new QSpacerItem (0, em_pixels (0.5f), QSizePolicy::Expanding, QSizePolicy::Fixed), 2, 0);
			tab_layout->addWidget (handled_modules_info, 3, 0);
			tab_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
			tab_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 4, 0);
		}
	}

	auto* widget_layout = new QGridLayout (widget);
	widget_layout->setMargin (0);
	widget_layout->addWidget (tabs, 0, 0);
	widget_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
	widget_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 0);

	return widget;
}

} // namespace xf::configurator

