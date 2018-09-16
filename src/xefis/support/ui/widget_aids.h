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

#ifndef XEFIS__SUPPORT__UI__WIDGET_AIDS_H__INCLUDED
#define XEFIS__SUPPORT__UI__WIDGET_AIDS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qutils.h>


namespace xf {

class WidgetAids: virtual public QWidget
{
  public:
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
	em_pixels (float ems) const;
};


inline si::PixelDensity
WidgetAids::pixel_density() const
{
	return si::PixelDensity (this->logicalDpiY());
}


inline float
WidgetAids::pixels (si::Length width) const
{
	return xf::pixels (width, pixel_density());
}


inline float
WidgetAids::em_pixels (float ems) const
{
	int v = font().pixelSize();

	if (v != -1)
		return ems * v;
	else
		return ems * font().pointSize() * xf::pixels_per_point (pixel_density());
}

} // namespace xf

#endif

