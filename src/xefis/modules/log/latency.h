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

#ifndef XEFIS__MODULES__LOG__LATENCY_H__INCLUDED
#define XEFIS__MODULES__LOG__LATENCY_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/module_io.h>
#include <xefis/core/xefis.h>


class Latency:
	public QObject,
	public v2::Module<>
{
	Q_OBJECT

  public:
	// Ctor
	explicit
	Latency (xf::Xefis*, std::string const& instance = {});

  private slots:
	/**
	 * Log latencies on module log.
	 */
	void
	log_latency();

  private:
	xf::Xefis*	_xefis;
	QTimer*		_log_timer;
};

#endif
