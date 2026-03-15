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

#ifndef XEFIS__MODULES__COMM__UDP_TRANSCEIVER_WIDGET_H__INCLUDED
#define XEFIS__MODULES__COMM__UDP_TRANSCEIVER_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/components/module_configurator/config_widget.h>


namespace xf {

class HistogramStatsWidget;
class HistogramWidget;

} // namespace xf

class QTimer;
class QWidget;
class UDPTransceiver;


class UDPTransceiverWidget: public xf::configurator::ConfigWidget
{
  public:
	explicit
	UDPTransceiverWidget (UDPTransceiver&, QWidget* parent = nullptr);

  private:
	void
	refresh();

	UDPTransceiver&				_udp_transceiver;
	QTimer*						_refresh_timer						{ nullptr };
	xf::HistogramWidget*		_received_bandwidth_histogram		{ nullptr };
	xf::HistogramStatsWidget*	_received_bandwidth_stats			{ nullptr };
	xf::HistogramWidget*		_transmitted_bandwidth_histogram	{ nullptr };
	xf::HistogramStatsWidget*	_transmitted_bandwidth_stats		{ nullptr };
};

#endif
