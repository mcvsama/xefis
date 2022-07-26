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

#ifndef XEFIS__MODULES__SYSTEMS__WATCHDOG_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__WATCHDOG_H__INCLUDED

// Xefis:
#include <xefis/app/xefis.h>
#include <xefis/config/all.h>
#include <xefis/core/module.h>

// Neutrino:
#include <neutrino/logger.h>

// Qt:
#include <QtCore/QSocketNotifier>

// Standard:
#include <cstddef>


class Watchdog: public xf::Module
{
  private:
	static constexpr char kLoggerScope[] = "mod::Watchdog";

  public:
	// Ctor
	explicit
	Watchdog (xf::Xefis*, xf::Logger const&, std::string_view const& instance = {});

  private:
	/**
	 * Ping arrived.
	 */
	void
	ping();

  private:
	xf::Logger							_logger;
	std::unique_ptr<QSocketNotifier>	_notifier;
	bool								_enabled			= false;
	int									_watchdog_write_fd	= 0;
	int									_watchdog_read_fd	= 0;
};

#endif
