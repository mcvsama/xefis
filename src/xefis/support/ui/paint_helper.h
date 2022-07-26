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
#include <QPaintDevice>
#include <QPalette>

// Standard:
#include <cstddef>


namespace xf {

class PaintHelper
{
  public:
	// Ctor
	explicit
	PaintHelper (QPaintDevice const& canvas, QPalette = {}, QFont = {});

	/**
	 * Return current PixelDensity.
	 */
	si::PixelDensity
	pixel_density() const;

	/**
	 * Return number of pixels that correspond to the given real length on the screen.
	 */
	float
	pixels (si::Length) const;

	/**
	 * Return number of pixels that correspond to the given line-heights of text.
	 */
	float
	em_pixels (float ems = 1.0f) const;

	/**
	 * Setup painter with antialiasing and other typical features used in Xefis.
	 */
	static void
	setup_painter (QPainter&);

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

} // namespace xf

#endif

