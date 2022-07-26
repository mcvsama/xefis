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

#ifndef XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__SCREEN_WIDGET_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__SCREEN_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/screen.h>
#include <xefis/core/components/module_configurator/config_widget.h>
#include <xefis/support/ui/histogram_widget.h>
#include <xefis/support/ui/histogram_stats_widget.h>
#include <xefis/support/ui/widget.h>

// Qt:
#include <QTimer>
#include <QWidget>

// Standard:
#include <cstddef>
#include <unordered_map>


namespace xf::configurator {

/**
 * Configuration widget for a Screen.
 */
class ScreenWidget: public ConfigWidget
{
	class Widgets
	{
	  public:
		WorkPerformer const*		work_performer			{ nullptr };
		// List of modules that use given WorkPerformer:
		std::string					module_names;
		xf::HistogramWidget*		start_latency_histogram	{ nullptr };
		xf::HistogramStatsWidget*	start_latency_stats		{ nullptr };
		QWidget*					start_latency_group		{ nullptr };
		xf::HistogramWidget*		total_latency_histogram	{ nullptr };
		xf::HistogramStatsWidget*	total_latency_stats		{ nullptr };
		QWidget*					total_latency_group		{ nullptr };
	};

  public:
	// Ctor
	explicit
	ScreenWidget (Screen&, QWidget* parent);

  private:
	void
	refresh();

	QWidget*
	create_performance_tab();

  private:
	Screen&						_screen;
	xf::HistogramWidget*		_painting_time_histogram	{ nullptr };
	xf::HistogramStatsWidget*	_painting_time_stats		{ nullptr };
	QTimer*						_refresh_timer;
	std::unordered_map<WorkPerformer const*, Widgets>
								_work_performer_widgets		{ 10 };
};

} // namespace xf::configurator

#endif

