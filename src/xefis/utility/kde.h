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

#ifndef XEFIS__UTILITY__KDE_H__INCLUDED
#define XEFIS__UTILITY__KDE_H__INCLUDED

// Lib:
#include <QWidget>

// Standard:
#include <cstddef>


namespace xf {

void
set_kde_blur_background (QWidget&, bool enabled);

} // namespace xf

#endif

