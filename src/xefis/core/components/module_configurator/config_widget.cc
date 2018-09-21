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
#include <QGroupBox>
#include <QMargins>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "config_widget.h"


namespace xf::configurator {

std::tuple<xf::HistogramWidget*, xf::HistogramStatsWidget*, QWidget*>
ConfigWidget::create_performance_widget (QWidget* parent, QString const& title) const
{
	QMargins const margins (em_pixels (0.5f), em_pixels (0.25f), em_pixels (0.5f), em_pixels (0.25f));

	auto* group_box = new QGroupBox (title, parent);
	group_box->setFixedSize (em_pixels (40.0f), em_pixels (15.0f));

	auto* histogram_widget = new xf::HistogramWidget (group_box);
	histogram_widget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

	auto* stats_widget = new xf::HistogramStatsWidget (group_box);

	auto* group_layout = new QVBoxLayout (group_box);
	group_layout->setMargin (0);
	group_layout->addWidget (histogram_widget);
	group_layout->addWidget (stats_widget);
	group_layout->setContentsMargins (margins);

	return { histogram_widget, stats_widget, group_box };
}

} // namespace xf::configurator

