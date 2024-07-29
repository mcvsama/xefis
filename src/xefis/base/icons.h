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

#ifndef XEFIS__BASE__ICONS_H__INCLUDED
#define XEFIS__BASE__ICONS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QtGui/QPixmap>


namespace xf::icons {

QPixmap
from_png_file (QString const& png_file) noexcept;

QPixmap
fron_svg_file (QString const& svg_file) noexcept;


#define XEFIS_PNG_ICON(key, file)		\
	static inline QPixmap key()			\
	{									\
		return from_png_file (XEFIS_SHARED_DIRECTORY "/images/16/" file);	\
	}

#define XEFIS_SVG_ICON(key, file)		\
	static inline QPixmap key()			\
	{									\
		return from_svg_file (XEFIS_SHARED_DIRECTORY "/images/16/" file);	\
	}


XEFIS_PNG_ICON (null, "null.png")
XEFIS_PNG_ICON (start, "start.png")
XEFIS_PNG_ICON (pause, "pause.png")
XEFIS_PNG_ICON (socket_dir, "socket-dir.png")
XEFIS_PNG_ICON (socket_value, "socket-value.png")
XEFIS_PNG_ICON (led_green_on, "led-green-on.png")
XEFIS_PNG_ICON (led_amber_on, "led-amber-on.png")
XEFIS_PNG_ICON (led_red_on, "led-red-on.png")
XEFIS_PNG_ICON (led_white_on, "led-white-on.png")
XEFIS_PNG_ICON (led_blue_on, "led-blue-on.png")
XEFIS_PNG_ICON (led_off, "led-off.png")
XEFIS_PNG_ICON (body, "body.png")
XEFIS_PNG_ICON (gravitating_body, "gravitating-body.png")
XEFIS_PNG_ICON (followed_body, "followed-body.png")
XEFIS_PNG_ICON (followed_gravitating_body, "followed-gravitating-body.png")
XEFIS_PNG_ICON (constraint, "constraint.png")

} // namespace xf::icons

#endif

