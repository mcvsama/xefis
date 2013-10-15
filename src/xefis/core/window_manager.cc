/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include <xefis/core/window.h>

// Local:
#include "window_manager.h"


namespace Xefis {

WindowManager::WindowManager (Application* application):
	_application (application)
{ }


void
WindowManager::add_window (Window* window)
{
	_windows.insert (window);
}


void
WindowManager::data_updated (Time const& update_time)
{
	for (Window* window: _windows)
		window->data_updated (update_time);
}

} // namespace Xefis

