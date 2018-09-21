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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/module_configurator/config_widget.h>
#include <xefis/utility/histogram.h>

// Local:
#include "processing_loop_widget.h"


namespace xf::configurator {

ProcessingLoopWidget::ProcessingLoopWidget (ProcessingLoop& processing_loop, QWidget* parent):
	ConfigWidget (parent),
	_processing_loop (processing_loop)
{
	auto* name_label = create_colored_strip_label (QString::fromStdString (_processing_loop.instance()).toHtmlEscaped(), QColor (0xec, 0xec, 0), Qt::AlignBottom, this);

	auto tabs = new QTabWidget (this);
	tabs->addTab (create_performance_tab(), "Performance");

	auto* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (name_label);
	layout->addWidget (tabs);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setSingleShot (false);
	_refresh_timer->setInterval (1000_Hz / ConfigWidget::kDataRefreshRate);
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &ProcessingLoopWidget::refresh);
	_refresh_timer->start();
}


void
ProcessingLoopWidget::refresh()
{
	{
		auto const& samples = _processing_loop.processing_times();
		// TODO auto histogram width (see __TODO__)
		xf::Histogram<si::Quantity<si::Millisecond>> histogram (samples.begin(), samples.end(), 0.01_ms, 0.0_ms, 1_ms);

		_processing_time_histogram->set_data (histogram, { _processing_loop.loop_period() });
		_processing_time_stats->set_data (histogram, std::make_optional<si::Quantity<si::Millisecond>> (_processing_loop.loop_period()));
	}

	{
		auto const& samples = _processing_loop.processing_latencies();
		// TODO auto histogram width (see __TODO__)
		xf::Histogram<si::Quantity<si::Millisecond>> histogram (samples.begin(), samples.end(), 0.2_ms, -10_ms, 10_ms);

		_processing_latency_histogram->set_data (histogram, { -_processing_loop.loop_period(), _processing_loop.loop_period() });
		_processing_latency_stats->set_data (histogram);
	}
}


QWidget*
ProcessingLoopWidget::create_performance_tab()
{
	auto* widget = new QWidget (this);
	QWidget* processing_time_group {};
	QWidget* processing_latency_group {};

	std::tie (_processing_time_histogram, _processing_time_stats, processing_time_group) = create_performance_widget (widget, "Processing time");
	std::tie (_processing_latency_histogram, _processing_latency_stats, processing_latency_group) = create_performance_widget (widget, "Processing latency");

	auto layout = new QGridLayout (widget);
	layout->setMargin (0);
	layout->addWidget (processing_time_group, 0, 0);
	layout->addWidget (processing_latency_group, 1, 0);

	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 2, 0);

	return widget;
}

} // namespace xf::configurator
