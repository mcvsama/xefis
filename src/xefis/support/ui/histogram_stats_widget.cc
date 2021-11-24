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
#include <QLabel>
#include <QWidget>
#include <QGridLayout>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "histogram_stats_widget.h"


namespace xf {

HistogramStatsWidget::HistogramStatsWidget (QWidget* parent):
	xf::Widget (parent)
{
	auto* num_samples_label = new QLabel ("Samples: ", this);
	auto* min_label = new QLabel ("Min: ", this);
	auto* max_label = new QLabel ("Max: ", this);
	auto* mean_label = new QLabel ("Mean: ", this);
	auto* median_label = new QLabel ("Median: ", this);
	auto* stddev_label = new QLabel ("σ: ", this);
	_critical_label = new QLabel ("", this);

	_num_samples_value = new QLabel (this);
	_min_value = new QLabel (this);
	_max_value = new QLabel (this);
	_mean_value = new QLabel (this);
	_median_value = new QLabel (this);
	_stddev_value = new QLabel (this);
	_critical_value = new QLabel (this);

	auto* layout = new QGridLayout (this);
	layout->setHorizontalSpacing (em_pixels (1.0f));

	layout->addWidget (num_samples_label, 0, 0);
	layout->addWidget (_num_samples_value, 0, 1);

	layout->addWidget (min_label, 1, 0);
	layout->addWidget (_min_value, 1, 1);

	layout->addWidget (max_label, 2, 0);
	layout->addWidget (_max_value, 2, 1);

	layout->addWidget (mean_label, 1, 2);
	layout->addWidget (_mean_value, 1, 3);

	layout->addWidget (median_label, 2, 2);
	layout->addWidget (_median_value, 2, 3);

	layout->addWidget (stddev_label, 1, 4);
	layout->addWidget (_stddev_value, 1, 5);

	layout->addWidget (_critical_label, 2, 4);
	layout->addWidget (_critical_value, 2, 5);

	layout->setColumnStretch (0, 0);
	layout->setColumnStretch (1, 100);
	layout->setColumnStretch (2, 0);
	layout->setColumnStretch (3, 100);
	layout->setColumnStretch (4, 0);
	layout->setColumnStretch (5, 100);
}

} // namespace xf

