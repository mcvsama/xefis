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

// Local:
#include "shadow_painter.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/responsibility.h>

// Standard:
#include <cstddef>


namespace xf {

void
ShadowPainter::paint (Shadow const& shadow, PaintFunction paint_function)
{
	{
		auto saved_pen = pen();
		Responsibility pen_restore ([&] { setPen (saved_pen); });

		auto const new_width = shadow.width_for_pen (saved_pen);
		auto const old_width = saved_pen.widthF();

		QPen new_pen = saved_pen;
		new_pen.setColor (shadow.color());
		new_pen.setWidthF (new_width);
		{
			auto dash_pattern = saved_pen.dashPattern();
			auto const factor = old_width / new_width;

			for (auto& v: dash_pattern)
				v *= factor;

			new_pen.setDashPattern (dash_pattern);
		}
		setPen (new_pen);

		paint_function (true);
	}

	paint_function (false);
}

} // namespace xf

