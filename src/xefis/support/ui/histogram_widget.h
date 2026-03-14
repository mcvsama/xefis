/* vim:ts=4
 *
 * Copyleft 2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__UI__HISTOGRAM_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__UI__HISTOGRAM_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/canvas_widget.h>

// Neutrino:
#include <neutrino/format.h>
#include <neutrino/math/histogram.h>
#include <neutrino/math/normal_distribution.h>
#include <neutrino/numeric.h>
#include <neutrino/qt/qstring.h>

// Qt:
#include <QWidget>

// Standard:
#include <cstddef>
#include <functional>
#include <numeric>
#include <optional>
#include <vector>


namespace xf {

class HistogramWidget: public CanvasWidget
{
  public:
	enum class Style
	{
		Line,
		Bars,
	};

  public:
	// Ctor
	explicit
	HistogramWidget (QWidget* parent);

	/**
	 * Set histogram to draw.
	 */
	template<class Value>
		void
		set_data (math::Histogram<Value> const&, std::vector<Value> marks = {});

	template<class Value, class Formatter>
		void
		set_data (math::Histogram<Value> const&, Formatter&& formatter, std::vector<Value> marks = {});

	/**
	 * Set number of helper lines in the grid.
	 */
	void
	set_grid_lines (std::size_t number);

	/**
	 * Show/hide count on the Y-axis.
	 */
	void
	set_y_legend_visible (bool);

	/**
	 * Set histogram style.
	 */
	void
	set_style (Style);

  protected:
	void
	update_canvas() override;

  private:
	Style						_style				{ Style::Bars };
	std::vector<float>			_marks;
	std::vector<std::size_t>	_bins;
	std::size_t					_binned_samples		{ 0 };
	std::size_t					_max_y				{ 0 };
	QString						_min_x_str;
	QString						_mid_x_str;
	QString						_max_x_str;
	QString						_max_y_str;
	std::optional<nu::math::NormalDistribution<float>>
								_normal_distribution;
	std::size_t					_grid_lines			{ 10 };
	bool						_y_legend_visible	{ false };
};


template<class Value>
	inline void
	HistogramWidget::set_data (math::Histogram<Value> const& histogram, std::vector<Value> marks)
	{
		set_data (histogram, [](Value const value) {
			return nu::format_unit (value, 3);
		}, std::move (marks));
	}


template<class Value, class Formatter>
	inline void
	HistogramWidget::set_data (math::Histogram<Value> const& histogram, Formatter&& formatter, std::vector<Value> marks)
	{
		_bins = histogram.bins();
		_binned_samples = std::accumulate (_bins.begin(), _bins.end(), std::size_t (0));
		_max_y = histogram.max_y();
		_min_x_str = nu::to_qstring (std::invoke (formatter, histogram.min_x()));
		_mid_x_str = nu::to_qstring (std::invoke (formatter, 0.5f * (histogram.min_x() + histogram.max_x())));
		_max_x_str = nu::to_qstring (std::invoke (formatter, histogram.max_x()));
		_max_y_str = nu::to_qstring (std::format ("{}", histogram.max_y()));
		_normal_distribution = std::nullopt;

		if (histogram.stddev() > Value())
		{
			auto const mean_x = renormalize (histogram.mean(),
											 nu::Range { histogram.min_x(), histogram.max_x() },
											 nu::Range { 0.0f, 1.0f });
			auto const mean_plus_stddev_x = renormalize (histogram.mean() + histogram.stddev(),
														 nu::Range { histogram.min_x(), histogram.max_x() },
														 nu::Range { 0.0f, 1.0f });
			auto const stddev_x = mean_plus_stddev_x - mean_x;

			if (stddev_x > 0.0f)
				_normal_distribution = nu::math::NormalDistribution<float> (mean_x, stddev_x);
		}

		_marks.reserve (marks.size());
		_marks.clear();

		for (auto const& m: marks)
		{
			auto const pos = renormalize (m,
										  nu::Range { histogram.min_x(), histogram.max_x() },
										  nu::Range { 0.0f, 1.0f });

			if (0.0f <= pos && pos <= 1.0f)
				_marks.emplace_back (pos);
		}

		mark_dirty();
		update();
	}

} // namespace xf

#endif
