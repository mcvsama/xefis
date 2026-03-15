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
#include "udp_transceiver_widget.h"

#include "udp_transceiver.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/paint_helper.h>
#include <xefis/support/ui/performance_widget.h>

// Neutrino:
#include <neutrino/math/histogram.h>
#include <neutrino/numeric.h>

// Lib:
#include <gsl/gsl_util>

// Qt:
#include <QGridLayout>
#include <QTimer>
#include <QWidget>

// Standard:
#include <algorithm>
#include <cstdint>


namespace {

void
update_bandwidth_histogram (std::vector<UDPTransceiver::Bandwidth> const& samples,
							xf::HistogramWidget& histogram_widget,
							xf::HistogramStatsWidget& stats_widget)
{
	if (samples.empty())
		return;

	auto const max_sample = std::max (UDPTransceiver::Bandwidth (1.0), *std::max_element (samples.begin(), samples.end()));
	auto const [range, grid_lines] = nu::get_max_for_axis (max_sample);
	auto const histogram = nu::math::Histogram<UDPTransceiver::Bandwidth> (samples.begin(), samples.end(), {
		.bin_width = range / 100.0,
		.min_x = UDPTransceiver::Bandwidth(),
		.max_x = range,
	});
	auto const format_bandwidth = [](UDPTransceiver::Bandwidth const bandwidth) {
		return xf::BandwidthSampler::format_bandwidth (bandwidth, 3);
	};

	histogram_widget.set_data (histogram, format_bandwidth);
	histogram_widget.set_grid_lines (grid_lines);
	stats_widget.set_data (histogram, format_bandwidth);
}

} // namespace


UDPTransceiverWidget::UDPTransceiverWidget (UDPTransceiver& udp_transceiver, QWidget* parent):
	ConfigWidget (parent),
	_udp_transceiver (udp_transceiver)
{
	auto const ph = xf::PaintHelper (*this);
	QWidget* received_bandwidth_group{};
	QWidget* transmitted_bandwidth_group{};

	std::tie (_received_bandwidth_histogram, _received_bandwidth_stats, received_bandwidth_group)
		= xf::create_performance_widget (this, "Received bandwidth");
	std::tie (_transmitted_bandwidth_histogram, _transmitted_bandwidth_stats, transmitted_bandwidth_group)
		= xf::create_performance_widget (this, "Transmitted bandwidth");

	auto* layout = new QGridLayout (this);
	layout->setContentsMargins (ph.widget_margins());
	layout->addWidget (received_bandwidth_group, 0, 0);
	layout->addWidget (transmitted_bandwidth_group, 1, 0);
	layout->addItem (ph.new_expanding_horizontal_spacer(), 0, 1);
	layout->addItem (ph.new_expanding_vertical_spacer(), 2, 0);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setSingleShot (false);
	_refresh_timer->setInterval (gsl::narrow<int> ((1.0 / ConfigWidget::kDataRefreshRate).in<si::Millisecond>()));
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &UDPTransceiverWidget::refresh);
	_refresh_timer->start();

	refresh();
}


void
UDPTransceiverWidget::refresh()
{
	auto const snapshot = _udp_transceiver.bandwidth_snapshot();
	update_bandwidth_histogram (snapshot.received_samples, *_received_bandwidth_histogram, *_received_bandwidth_stats);
	update_bandwidth_histogram (snapshot.transmitted_samples, *_transmitted_bandwidth_histogram, *_transmitted_bandwidth_stats);
}
