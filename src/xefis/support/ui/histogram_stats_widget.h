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

#ifndef XEFIS__SUPPORT__UI__HISTOGRAM_STATS_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__UI__HISTOGRAM_STATS_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/widget.h>

// Neutrino:
#include <neutrino/math/histogram.h>

// Qt:
#include <QWidget>
#include <QLabel>

// Lib:
#include <boost/lexical_cast.hpp>

// Standard:
#include <cstddef>
#include <optional>
#include <vector>


namespace xf {

class HistogramStatsWidget: public xf::Widget
{
  public:
	// Ctor
	explicit
	HistogramStatsWidget (QWidget* parent);

	/**
	 * Set histogram to use for stats.
	 */
	template<class HistogramValue, class CriticalValue = HistogramValue>
		void
		set_data (Histogram<HistogramValue> const&, std::optional<CriticalValue> critical_value = {});

  private:
	QLabel*	_num_samples_value;
	QLabel*	_min_value;
	QLabel*	_max_value;
	QLabel*	_mean_value;
	QLabel*	_median_value;
	QLabel*	_stddev_value;
	QLabel*	_critical_label;
	QLabel*	_critical_value;
};


template<class HistogramValue, class CriticalValue>
	inline void
	HistogramStatsWidget::set_data (Histogram<HistogramValue> const& histogram, std::optional<CriticalValue> critical_value)
	{
		_num_samples_value->setText (QString::number (histogram.n_samples()));
		_min_value->setText (QString::fromStdString (boost::lexical_cast<std::string> (histogram.min())));
		_max_value->setText (QString::fromStdString (boost::lexical_cast<std::string> (histogram.max())));
		_mean_value->setText (QString::fromStdString (boost::lexical_cast<std::string> (histogram.mean())));
		_median_value->setText (QString::fromStdString (boost::lexical_cast<std::string> (histogram.median())));
		_stddev_value->setText (QString::fromStdString (boost::lexical_cast<std::string> (histogram.stddev())));

		if (critical_value)
		{
			_critical_label->setText (QString::fromStdString ("> " + boost::lexical_cast<std::string> (*critical_value) + ": "));
			_critical_value->setText (QString::fromStdString (boost::lexical_cast<std::string> (100 * histogram.normalized_percentile_for (*critical_value))) + "%");
		}
		else
		{
			_critical_label->setText ("");
			_critical_value->setText ("");
		}
	}

} // namespace xf

#endif

