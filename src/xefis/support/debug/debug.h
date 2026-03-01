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

#ifndef XEFIS__SUPPORT__DEBUG__DEBUG_H__INCLUDED
#define XEFIS__SUPPORT__DEBUG__DEBUG_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QLayout>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Get layout for the debug window, so that another debug-widget can be added.
 * If debug window doesn't exist, it's created.
 */
QVBoxLayout&
get_debug_window_layout();

} // namespace xf

#endif
