/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__SYSYTEM_H__INCLUDED
#define XEFIS__CORE__SYSYTEM_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/hardware/serial_port.h>


namespace Xefis {

class System
{
  public:
	explicit System();

	/**
	 * Set system clock.
	 * Return true on success.
	 */
	bool
	set_clock (Time const& unix_time);

	template<class ...Args>
		Unique<SerialPort>
		allocate_serial_port (Args&& ...args)
		{
			return std::make_unique<SerialPort> (std::forward<Args> (args)...);
		}

  private:
	Logger _logger;
};

} // namespace Xefis

#endif

