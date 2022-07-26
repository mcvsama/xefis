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

#ifndef XEFIS__CORE__SYSTEM_H__INCLUDED
#define XEFIS__CORE__SYSTEM_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/bus/serial_port.h>
#include <neutrino/logger.h>

// Standard:
#include <cstddef>


namespace xf {

class System
{
  public:
	// Ctor
	explicit
	System (Logger const&);

	// Dtor
	~System();

	/**
	 * Set system clock.
	 * Return true on success.
	 */
	bool
	set_clock (si::Time unix_time);

  private:
	Logger _logger;
};

} // namespace xf

#endif

