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
#include <xefis/core/v1/window.h>

// Local:
#include "window_manager.h"


namespace xf {

WindowManager::WindowManager()
{
	_logger.set_prefix ("<window manager>");
	_logger << "Creating WindowManager" << std::endl;
}


WindowManager::~WindowManager()
{
	_logger << "Destroying WindowManager" << std::endl;
}


void
WindowManager::add_window (Unique<Window> window)
{
	_windows.insert (std::move (window));
}


void
WindowManager::data_updated (Time const& update_time)
{
	for (auto const& window: _windows)
		window->data_updated (update_time);
}

} // namespace xf

