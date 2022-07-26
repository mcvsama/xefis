/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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
#include "kde.h"

// Lib:
#include <X11/Xlib.h>
#include <X11/Xatom.h>

// Qt:
#include <QX11Info>
#include <QWidget>

// Standard:
#include <cstddef>


namespace xf {

void
set_kde_blur_background (QWidget& widget, bool enabled)
{
	auto* x_display = QX11Info::display();
	auto blur_atom = XInternAtom (x_display, "_KDE_NET_WM_BLUR_BEHIND_REGION", False);

	widget.setAttribute (Qt::WA_TranslucentBackground, enabled);
	widget.setAttribute (Qt::WA_NoSystemBackground, enabled);

	if (enabled)
		XChangeProperty (x_display, widget.winId(), blur_atom, XA_CARDINAL, 32, PropModeReplace, 0, 0);
	else
		XDeleteProperty (x_display, widget.winId(), blur_atom);

	widget.update();
}

} // namespace xf

