/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__UI__PAINTER__ARROW_H__INCLUDED
#define XEFIS__SUPPORT__UI__PAINTER__ARROW_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QPainter>
#include <QPoint>

// Standard:
#include <cstddef>


namespace xf {

void
draw_arrow (QPainter&, QPointF const& from, QPointF const& to, double arrowhead_size = 3.0);

} // namespace xf

#endif

