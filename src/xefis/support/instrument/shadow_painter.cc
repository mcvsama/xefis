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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/responsibility.h>

// Local:
#include "shadow_painter.h"


namespace xf {

void
ShadowPainter::paint (Shadow const& shadow, PaintFunction paint_function)
{
	{
		auto saved_pen = pen();
		Responsibility pen_restore ([&] { setPen (saved_pen); });

		QPen new_pen = saved_pen;
		new_pen.setColor (shadow.color());
		new_pen.setWidthF (shadow.width_for_pen (new_pen));
		setPen (new_pen);

		paint_function (true);
	}

	paint_function (false);
}

} // namespace xf

