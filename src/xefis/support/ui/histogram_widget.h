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

// Standard:
#include <cstddef>
#include <optional>

// Lib:
#include <boost/lexical_cast.hpp>

// Qt:
#include <QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/widget_aids.h>
#include <xefis/utility/histogram.h>


namespace xf {

class HistogramWidget:
	virtual public QWidget,
	public xf::WidgetAids
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
		set_data (Histogram<Value> const&);

  protected:
	void
	update_canvas();

	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

  private:
	Style						_style		{ Style::Bars };
	std::optional<QImage>		_canvas;
	std::vector<std::size_t>	_bins;
	std::size_t					_y_max;
	QString						_x_min_str;
	QString						_x_max_str;
	QString						_y_max_str;
};


template<class Value>
	inline void
	HistogramWidget::set_data (Histogram<Value> const& histogram)
	{
		using std::to_string;

		_y_max = histogram.y_max();
		_x_min_str = QString::fromStdString (boost::lexical_cast<std::string> (histogram.x_min()));
		_x_max_str = QString::fromStdString (boost::lexical_cast<std::string> (histogram.x_max()));
		_y_max_str = QString::fromStdString (boost::lexical_cast<std::string> (histogram.y_max()));
		_bins = histogram.bins();
		_canvas.reset();
		update();
	}

} // namespace xf

#endif

