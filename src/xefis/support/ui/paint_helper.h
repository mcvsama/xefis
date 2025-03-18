/* vim:ts=4
 *
 * Copyleft 2022  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__UI__PAINT_HELPER_H__INCLUDED
#define XEFIS__SUPPORT__UI__PAINT_HELPER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/qt/qutils.h>
#include <neutrino/si/si.h>

// Qt:
#include <QFont>
#include <QFrame>
#include <QLabel>
#include <QMargins>
#include <QPaintDevice>
#include <QPalette>
#include <QSpacerItem>
#include <QWidget>

// Standard:
#include <cstddef>


namespace xf {

class PaintHelper
{
  public:
	// Ctor
	explicit
	PaintHelper (QPaintDevice const& canvas, QPalette = {}, QFont = {});

	// Ctor
	explicit
	PaintHelper (QWidget const&);

	/**
	 * Return current PixelDensity.
	 */
	[[nodiscard]]
	si::PixelDensity
	pixel_density() const;

	/**
	 * Return number of pixels that correspond to the given real length on the screen.
	 */
	[[nodiscard]]
	float
	pixels (si::Length) const;

	/**
	 * Return number of pixels that correspond to the given line-heights of text.
	 */
	[[nodiscard]]
	float
	em_pixels (float ems = 1.0f) const;

	/**
	 * Like em_pixels(), but rounds to integers.
	 */
	[[nodiscard]]
	int
	em_pixels_int (float ems = 1.0f) const
		{ return static_cast<int> (0.5 + em_pixels (ems)); }

	/**
	 * Setup painter with antialiasing and other typical features used in Xefis.
	 */
	static void
	setup_painter (QPainter&);

	[[nodiscard]]
	QSpacerItem*
	new_fixed_horizontal_spacer (float ems = 1.0f) const
		{ return new QSpacerItem (em_pixels_int (ems), 0, QSizePolicy::Fixed, QSizePolicy::Fixed); }

	[[nodiscard]]
	QSpacerItem*
	new_fixed_vertical_spacer (float ems = 1.0f) const
		{ return new QSpacerItem (0, em_pixels_int (ems), QSizePolicy::Fixed, QSizePolicy::Fixed); }

	[[nodiscard]]
	QSpacerItem*
	new_expanding_horizontal_spacer (float minimum_ems = 1.0f) const
		{ return new QSpacerItem (em_pixels_int (minimum_ems), 0, QSizePolicy::Expanding, QSizePolicy::Fixed); }

	[[nodiscard]]
	QSpacerItem*
	new_expanding_vertical_spacer (float minimum_ems = 1.0f) const
		{ return new QSpacerItem (0, em_pixels_int (minimum_ems), QSizePolicy::Fixed, QSizePolicy::Expanding); }

	[[nodiscard]]
	static QFrame*
	new_hline();

	[[nodiscard]]
	QMargins
	group_box_margins() const;

  private:
	QPaintDevice const&	_canvas;
	QPalette			_palette;
	QFont				_font;
};


inline si::PixelDensity
PaintHelper::pixel_density() const
{
	return si::PixelDensity (_canvas.logicalDpiY());
}


inline float
PaintHelper::pixels (si::Length width) const
{
	return xf::pixels (width, pixel_density());
}


inline float
PaintHelper::em_pixels (float ems) const
{
	int v = _font.pixelSize();

	if (v != -1)
		return ems * v;
	else
		return ems * _font.pointSize() * xf::pixels_per_point (pixel_density());
}


inline QFrame*
PaintHelper::new_hline()
{
	auto* line = new QFrame();
	line->setFrameShape (QFrame::HLine);
	return line;
}


inline QLabel*
align_right (QLabel* const label)
{
	label->setAlignment (Qt::AlignRight);
	return label;
};

} // namespace xf

#endif

