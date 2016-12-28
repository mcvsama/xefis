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

// System:
#include <sys/unistd.h>
#include <sys/fcntl.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/xefis.h>

// Local:
#include "watchdog.h"


Watchdog::Watchdog (xf::Xefis* xefis, std::string const& instance):
	Module (instance)
{
	Optional<int> watchdog_write_fd = xefis->options().watchdog_write_fd();
	Optional<int> watchdog_read_fd = xefis->options().watchdog_read_fd();

	if (!watchdog_write_fd || *watchdog_write_fd < 3)
	{
		log() << "Warning: watchdog disabled: invalid watchdog-write file descriptor." << std::endl;
		return;
	}

	if (!watchdog_read_fd || *watchdog_read_fd < 3)
	{
		log() << "Warning: watchdog disabled: invalid watchdog-read file descriptor." << std::endl;
		return;
	}

	_enabled = true;

	_watchdog_write_fd = *watchdog_write_fd;
	_watchdog_read_fd = *watchdog_read_fd;

	if (_enabled)
	{
		_notifier = std::make_unique<QSocketNotifier> (_watchdog_read_fd, QSocketNotifier::Read);
		_notifier->setEnabled (true);
		QObject::connect (_notifier.get(), &QSocketNotifier::activated, std::bind (&Watchdog::ping, this));
		// Read should be non-blocking:
		::fcntl (_watchdog_read_fd, F_SETFL, O_NONBLOCK);
	}
}


void
Watchdog::ping()
{
	uint8_t c;
	int n = 0;

	while ((n = ::read (_watchdog_read_fd, &c, 1)) > 0)
	{
		c ^= 0x55;
		::write (_watchdog_write_fd, &c, 1);
	}

	::fsync (_watchdog_write_fd);
}

