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

#ifndef XEFIS__SUPPORT__UI__RADIATION_PLOT_H__INCLUDED
#define XEFIS__SUPPORT__UI__RADIATION_PLOT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/antennas/antenna_model.h>
#include <xefis/support/ui/canvas_widget.h>

// Qt:
#include <QBrush>
#include <QPen>
#include <QPolygonF>
#include <QString>
#include <QWidget>

// Standard:
#include <cstddef>
#include <optional>


namespace xf {

class RadiationPlot: public CanvasWidget
{
  private:
	struct Pens
	{
		QPen	coordinate_lines_pen;
		QPen	coordinate_circles_pen;
		QPen	radiation_pattern_pen;
		QBrush	radiation_pattern_brush;
	};

	struct ThemeColors
	{
		QColor	background;
		QColor	coordinate_lines;
		QColor	coordinate_circles;
		QColor	pattern_fill;
		QColor	pattern_outline;
	};

  public:
	// Ctor
	explicit
	RadiationPlot (QWidget* parent = nullptr, Qt::WindowFlags = Qt::Widget);

	/**
	 * Set antenna model to visualize.
	 */
	void
	set_antenna_model (AntennaModel const&);

	/**
	 * Set view axis vector (normal to the plot plane).
	 * The plot shows radiation in the plane perpendicular to this vector.
	 * A zero vector falls back to +Z axis.
	 */
	void
	set_axis_vector (SpaceVector<double, BodyOrigin> const&);

	/**
	 * Set number of angular samples used to draw the polar curve.
	 */
	void
	set_num_samples (std::size_t);

  protected:
	// QWidget/CanvasWidget API
	void
	resizeEvent (QResizeEvent*) override;

	// QWidget API
	void
	changeEvent (QEvent*) override;

	// CanvasWidget API
	void
	update_canvas() override;

  private:
	void
	prepare_for_painting();

	void
	update_pens();

	[[nodiscard]]
	static ThemeColors
	theme_colors_for (QPalette const&, QPalette::ColorGroup);

	[[nodiscard]]
	static QString
	axis_label_for (SpaceVector<double, BodyOrigin> const&);

  private:
	AntennaModel const*				_antenna_model		{ nullptr };
	SpaceVector<double, BodyOrigin>	_axis_vector		{ 0.0, 0.0, 1.0 };
	std::size_t						_num_samples		{ 180 };
	QPolygonF						_radiation_pattern_polygon;
	QString							_horizontal_positive_axis_label;
	QString							_horizontal_negative_axis_label;
	QString							_vertical_positive_axis_label;
	QString							_vertical_negative_axis_label;
	QString							_view_axis_label;
	std::optional<Pens>				_pens;
};

} // namespace xf

#endif
