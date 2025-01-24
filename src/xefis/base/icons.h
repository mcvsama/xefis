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
		return from_png_file (XEFIS_SHARED_DIRECTORY "/images/" file);	\
	}

#define XEFIS_SVG_ICON(key, file)		\
	static inline QPixmap key()			\
	{									\
		return from_svg_file (XEFIS_SHARED_DIRECTORY "/images/" file);	\
	}


XEFIS_PNG_ICON (null, "icons/null.png")
XEFIS_PNG_ICON (start, "icons/start.png")
XEFIS_PNG_ICON (pause, "icons/pause.png")
XEFIS_PNG_ICON (socket_dir, "icons/socket-dir.png")
XEFIS_PNG_ICON (socket_value, "icons/socket-value.png")
XEFIS_PNG_ICON (led_green_on, "icons/led-green-on.png")
XEFIS_PNG_ICON (led_amber_on, "icons/led-amber-on.png")
XEFIS_PNG_ICON (led_red_on, "icons/led-red-on.png")
XEFIS_PNG_ICON (led_white_on, "icons/led-white-on.png")
XEFIS_PNG_ICON (led_blue_on, "icons/led-blue-on.png")
XEFIS_PNG_ICON (led_off, "icons/led-off.png")
XEFIS_PNG_ICON (group, "icons/group.png")
XEFIS_PNG_ICON (body, "icons/body.png")
XEFIS_PNG_ICON (gravitating_body, "icons/gravitating-body.png")
XEFIS_PNG_ICON (followed_body, "icons/followed-body.png")
XEFIS_PNG_ICON (followed_gravitating_body, "icons/followed-gravitating-body.png")
XEFIS_PNG_ICON (constraint, "icons/constraint.png")

XEFIS_PNG_ICON (digit_0, "digits/digit-0.png")
XEFIS_PNG_ICON (digit_1, "digits/digit-1.png")
XEFIS_PNG_ICON (digit_2, "digits/digit-2.png")
XEFIS_PNG_ICON (digit_3, "digits/digit-3.png")
XEFIS_PNG_ICON (digit_4, "digits/digit-4.png")
XEFIS_PNG_ICON (digit_5, "digits/digit-5.png")
XEFIS_PNG_ICON (digit_6, "digits/digit-6.png")
XEFIS_PNG_ICON (digit_7, "digits/digit-7.png")
XEFIS_PNG_ICON (digit_8, "digits/digit-8.png")
XEFIS_PNG_ICON (digit_9, "digits/digit-9.png")
XEFIS_PNG_ICON (digit_minus, "digits/digit-minus.png")
XEFIS_PNG_ICON (digit_empty, "digits/digit-empty.png")
XEFIS_PNG_ICON (digit_full, "digits/digit-full.png")
XEFIS_PNG_ICON (digit_dot, "digits/digit-dot.png")

} // namespace xf::icons

#endif

