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

// Local:
#include "watchdog.h"

// Xefis:
#include <xefis/app/xefis.h>
#include <xefis/config/all.h>

// System:
#include <sys/unistd.h>
#include <sys/fcntl.h>

// Standard:
#include <cstddef>


Watchdog::Watchdog (xf::Xefis* xefis, xf::Logger const& logger, std::string_view const& instance):
	Module (instance),
	_logger (logger.with_scope (std::string (kLoggerScope) + "#" + instance))
{
	std::optional<int> watchdog_write_fd = xefis->options().watchdog_write_fd;
	std::optional<int> watchdog_read_fd = xefis->options().watchdog_read_fd;

	if (!watchdog_write_fd || *watchdog_write_fd < 3)
	{
		_logger << "Warning: watchdog disabled: invalid watchdog-write file descriptor." << std::endl;
		return;
	}

	if (!watchdog_read_fd || *watchdog_read_fd < 3)
	{
		_logger << "Warning: watchdog disabled: invalid watchdog-read file descriptor." << std::endl;
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

		while ((n = ::write (_watchdog_write_fd, &c, 1)) != 1)
		{
			if (n == -1)
			{
				switch (errno)
				{
					if (errno == EINTR)
						continue;
					else if (errno == EPIPE)
						return;
					else
						_logger << "Error when writing pong: " << strerror (errno) << std::endl;
				}
			}
		}
	}

	::fsync (_watchdog_write_fd);
}

