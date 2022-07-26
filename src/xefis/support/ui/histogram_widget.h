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

#ifndef XEFIS__SUPPORT__UI__HISTOGRAM_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__UI__HISTOGRAM_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/canvas_widget.h>

// Neutrino:
#include <neutrino/math/histogram.h>
#include <neutrino/numeric.h>

// Qt:
#include <QWidget>

// Lib:
#include <boost/lexical_cast.hpp>

// Standard:
#include <cstddef>
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
		set_data (Histogram<Value> const&, std::vector<Value> marks = {});

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
	std::size_t					_y_max;
	QString						_x_min_str;
	QString						_x_mid_str;
	QString						_x_max_str;
	QString						_y_max_str;
	std::size_t					_grid_lines			{ 10 };
	bool						_y_legend_visible	{ false };
};


template<class Value>
	inline void
	HistogramWidget::set_data (Histogram<Value> const& histogram, std::vector<Value> marks)
	{
		_bins = histogram.bins();
		_y_max = histogram.y_max();
		_x_min_str = QString::fromStdString (boost::lexical_cast<std::string> (histogram.x_min()));
		_x_mid_str = QString::fromStdString (boost::lexical_cast<std::string> (0.5f * (histogram.x_min() + histogram.x_max())));
		_x_max_str = QString::fromStdString (boost::lexical_cast<std::string> (histogram.x_max()));
		_y_max_str = QString::fromStdString (boost::lexical_cast<std::string> (histogram.y_max()));

		_marks.reserve (marks.size());
		_marks.clear();

		for (auto const& m: marks)
		{
			auto const pos = renormalize (m, { histogram.x_min(), histogram.x_max() }, Range { 0.0f, 1.0f });

			if (0.0f <= pos && pos <= 1.0f)
				_marks.emplace_back (pos);
		}

		mark_dirty();
		update();
	}

} // namespace xf

#endif

