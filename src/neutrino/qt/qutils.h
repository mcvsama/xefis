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

#ifndef NEUTRINO__QT__QUTILS_H__INCLUDED
#define NEUTRINO__QT__QUTILS_H__INCLUDED

// Standard:
#include <cmath>
#include <cstddef>

// Qt:
#include <QSize>
#include <QTreeWidgetItem>

// Neutrino:
#include <neutrino/si/si.h>


namespace neutrino {

using namespace si::literals;


inline float
diagonal (QSize const& size)
{
	auto const w = size.width();
	auto const h = size.height();

	return std::sqrt (static_cast<float> (w * w + h * h));
}


inline float
diagonal (QSizeF const& size)
{
	auto const w = size.width();
	auto const h = size.height();

	return std::sqrt (static_cast<float> (w * w + h * h));
}


inline float
pixels (si::Length length, si::PixelDensity pixel_density)
{
	return length * pixel_density;
}


inline float
pixels_per_point (si::PixelDensity dpi)
{
	return dpi / (72 / 1_in);
}


/**
 * Return default font line-height in pixels.
 */
extern float
default_line_height (QWidget* = nullptr);


extern void
setup_appereance (QTreeWidgetItem&);

} // namespace neutrino

#endif

