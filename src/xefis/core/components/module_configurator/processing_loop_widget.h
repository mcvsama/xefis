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

#ifndef XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__PROCESSING_LOOP_WIDGET_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__PROCESSING_LOOP_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/processing_loop.h>
#include <xefis/core/components/module_configurator/config_widget.h>
#include <xefis/support/ui/histogram_widget.h>
#include <xefis/support/ui/histogram_stats_widget.h>
#include <xefis/support/ui/widget.h>

// Qt:
#include <QTimer>
#include <QWidget>

// Standard:
#include <cstddef>


namespace xf::configurator {

/**
 * Configuration widget for a ProcessingLoop.
 */
class ProcessingLoopWidget: public ConfigWidget
{
  public:
	// Ctor
	explicit
	ProcessingLoopWidget (ProcessingLoop&, QWidget* parent);

  private:
	void
	refresh();

	QWidget*
	create_performance_tab();

  private:
	ProcessingLoop&				_processing_loop;
	xf::HistogramWidget*		_communication_time_histogram	{ nullptr };
	xf::HistogramStatsWidget*	_communication_time_stats		{ nullptr };
	xf::HistogramWidget*		_processing_time_histogram		{ nullptr };
	xf::HistogramStatsWidget*	_processing_time_stats			{ nullptr };
	xf::HistogramWidget*		_processing_latency_histogram	{ nullptr };
	xf::HistogramStatsWidget*	_processing_latency_stats		{ nullptr };
	QTimer*						_refresh_timer;
};

} // namespace xf::configurator

#endif

