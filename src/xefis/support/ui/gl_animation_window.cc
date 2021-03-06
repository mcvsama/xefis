/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
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
#include "gl_animation_window.h"

// Qt:
#include <QResizeEvent>
#include <QScreen>

// Neutrino:
#include <neutrino/variant.h>


namespace xf {

GLAnimationWindow::GLAnimationWindow (QSize size, RefreshRate const refresh_rate, std::function<void (QOpenGLPaintDevice&)> const display_function):
	QWindow (static_cast<QWindow*> (nullptr)),
	_requested_refresh_rate (refresh_rate),
	_display_function (display_function)
{
	setSurfaceType (QWindow::OpenGLSurface);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setSingleShot (false);
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &GLAnimationWindow::refresh);
	set_refresh_rate (refresh_rate);
	_refresh_timer->start();

	setTitle ("Xefis");
	resize (size);
	create();
}


void
GLAnimationWindow::set_refresh_rate (RefreshRate const refresh_rate)
{
	_current_refresh_rate = std::visit (overload {
		[](si::Frequency const fps)
			{ return fps; },

		[this] (FPSMode const mode)
		{
			switch (mode)
			{
				case AutoFPS:
					// TODO update fps when:
					// 1. moving to another screen
					// 2. on signal refreshRateChanged()
					// 3. on show
					// 4. on hide (set to 1_Hz)
					if (auto* screen = this->screen())
						return 1_Hz * screen->refreshRate();
					break;
			}

			// Some sane default:
			return 60_Hz;
		},
	}, refresh_rate);

	_refresh_timer->setInterval ((1 / _current_refresh_rate).in<si::Millisecond>());
}


void
GLAnimationWindow::refresh()
{
	if (isExposed())
	{
		if (!_open_gl_context)
		{
			auto format = requestedFormat();
			format.setDepthBufferSize (24);
			format.setProfile (QSurfaceFormat::CoreProfile);
			// OpenGL antialiasing:
			format.setSamples (8);

			_open_gl_context = std::make_unique<QOpenGLContext> (this);
			_open_gl_context->setFormat (format);
			_open_gl_context->create();
		}

		if (_open_gl_context->makeCurrent (this))
		{
			if (!_open_gl_device)
				_open_gl_device = std::make_unique<QOpenGLPaintDevice>();

			_open_gl_device->setSize (size() * devicePixelRatio());
			_open_gl_device->setDevicePixelRatio (devicePixelRatio());

			// Paint black background, reset z-buffer and stencil buffer:
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			_display_function (*_open_gl_device);
			_open_gl_context->swapBuffers (this);
		}
		else
			std::cerr << "Could not make OpenGL context current.\n";
	}
}

} // namespace xf

