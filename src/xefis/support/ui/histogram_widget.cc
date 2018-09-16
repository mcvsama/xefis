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

// Lib:
#include <boost/range/adaptor/indexed.hpp>

// Qt:
#include <QImage>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "histogram_widget.h"


namespace xf {

HistogramWidget::HistogramWidget (QWidget* parent):
	QWidget (parent)
{ }


void
HistogramWidget::update_canvas()
{
	QPalette const pal = palette();
	QColor const axes_color = pal.foreground().color();
	QColor const line_color = pal.foreground().color();
	QColor const bar_color = pal.foreground().color();
	QColor fill_color = pal.foreground().color();
	fill_color.setAlpha (0x7f);

	if (!_canvas)
	{
		_canvas.emplace (size(), QImage::Format_ARGB32_Premultiplied);
		auto& canvas = *_canvas;

		QPainter painter (&canvas);
		painter.setRenderHint (QPainter::Antialiasing, true);
		painter.setRenderHint (QPainter::TextAntialiasing, true);
		painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
		painter.setRenderHint (QPainter::NonCosmeticDefaultPen, true);

		QFontMetricsF const font_metrics (font());
		auto const y_max_str_width = std::max (font_metrics.width (_y_max_str), font_metrics.width ("0000"));
		auto const axes_width = em_pixels (0.1f);
		auto const chart_width = em_pixels (0.05f);
		auto const text_height = font_metrics.height();
		auto const bug_length = em_pixels (0.4f);

		QRectF const drawable_rect = rect().adjusted (axes_width, axes_width, -axes_width, -axes_width);
		QRectF const axes_rect = drawable_rect.adjusted (y_max_str_width + bug_length, 0.5f * text_height, 0.0f, -(text_height + bug_length));
		QRectF const chart_rect = axes_rect.adjusted (0.5f * axes_width, 0.0f, 0.0f, -0.5f * axes_width);

		auto const n_bins = _bins.size();
		auto const inv_y_max = chart_rect.height() / _y_max;
		auto const bin_width = chart_rect.width() / n_bins;

		canvas.fill (pal.background().color());

		if (chart_rect.isValid())
		{
			painter.resetTransform();
			painter.translate (chart_rect.bottomLeft());
			painter.scale (1.0f, -1.0f);

			switch (_style)
			{
				case Style::Line:
				{
					QPolygonF line;
					line << QPointF (0.0f, 0.0f);

					for (auto const& bin: _bins | boost::adaptors::indexed (0))
						line << QPointF ((bin.index() + 0.5f) * bin_width, bin.value() * inv_y_max);

					line << QPointF (chart_rect.width(), 0.0f);

					painter.setPen (QPen (line_color, chart_width, Qt::SolidLine, Qt::RoundCap));
					painter.setBrush (fill_color);
					painter.drawPolygon (line);
					break;
				}

				case Style::Bars:
				{
					auto const bar_width = 0.8 * chart_rect.width() / n_bins;

					painter.setPen (QPen (bar_color, bar_width, Qt::SolidLine, Qt::FlatCap));

					for (auto const& bin: _bins | boost::adaptors::indexed (0))
					{
						auto const x = (bin.index() + 0.5f) * bin_width;

						painter.drawLine (QPointF (x, 0.0f), QPointF (x, bin.value() * inv_y_max));
					}
					break;
				}
			}
		}

		if (axes_rect.isValid())
		{
			painter.resetTransform();
			painter.setPen (QPen (axes_color, axes_width, Qt::SolidLine, Qt::RoundCap));
			// Axes:
			painter.drawLine (axes_rect.topLeft(), axes_rect.bottomLeft());
			painter.drawLine (axes_rect.bottomLeft(), axes_rect.bottomRight());
			// Top-value bug:
			painter.drawLine (axes_rect.topLeft(), axes_rect.topLeft() - QPointF (bug_length, 0.0f));
			// Top-value text:
			QRectF y_max_text_rect (rect().topLeft(), rect().topLeft() + QPointF (y_max_str_width, font_metrics.height()));
			painter.drawText (y_max_text_rect, Qt::AlignVCenter | Qt::AlignRight, _y_max_str);

			// Min/expected/max values:
			auto paint_x_value = [&] (QString const& text, std::size_t bin_number, Qt::Alignment alignment) {
				auto const x = axes_rect.left() + (bin_number + 0.5f) * bin_width;
				QRectF text_rect (QPointF (0, axes_rect.bottom() + bug_length), QSizeF (font_metrics.width (text), text_height));

				if (alignment & Qt::AlignRight)
					text_rect.moveRight (x);
				else
					text_rect.moveLeft (x);

				painter.drawLine (QPointF (x, axes_rect.bottom()), QPointF (x, axes_rect.bottom() + bug_length));
				painter.drawText (text_rect, Qt::AlignCenter, text);
			};

			paint_x_value (_x_min_str, 0u, Qt::AlignLeft);
			paint_x_value (_x_max_str, n_bins - 1, Qt::AlignRight);
		}
	}
}


void
HistogramWidget::resizeEvent (QResizeEvent*)
{
	_canvas.reset();
	update();
}


void
HistogramWidget::paintEvent (QPaintEvent* paint_event)
{
	update_canvas();

	QPainter painter (this);
	auto const rect = paint_event->region().boundingRect();
	painter.drawImage (rect, *_canvas, rect);
}

} // namespace xf

