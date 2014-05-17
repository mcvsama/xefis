/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__WINDOW_MANAGER_H__INCLUDED
#define XEFIS__CORE__WINDOW_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/application.h>
#include <xefis/core/window.h>


namespace Xefis {

class WindowManager: public QWidget
{
  public:
	// Ctor
	WindowManager (Application*);

	// Dtor
	~WindowManager();

	/**
	 * Add window to be managed.
	 */
	void
	add_window (Unique<Window>);

	/**
	 * Call data_updated() on all windows.
	 */
	void
	data_updated (Time const& update_time);

  private:
	std::set<Unique<Window>>	_windows;
};

} // namespace Xefis

#endif

