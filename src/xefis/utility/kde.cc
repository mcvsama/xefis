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

// Qt:
#include <QApplication>
#include <QGuiApplication>
#include <QWidget>

// Standard:
#include <cstddef>

// Because the Xlib API is completely fucked up and uses #defines instead of constants, these headers have to go last,
// otherwise there will be UB or compilation errors:
#include <X11/Xlib.h>
#include <X11/Xatom.h>


namespace xf {

void
set_kde_blur_background (QWidget& widget, bool enabled)
{
	auto* app = qobject_cast<QGuiApplication*> (QApplication::instance());
	auto* x11 = app->nativeInterface<QNativeInterface::QX11Application>();
	Display* x_display = x11->display();
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

