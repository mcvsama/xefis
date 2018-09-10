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
#include <xefis/core/accounting.h>
#include <xefis/core/logger.h>
#include <xefis/core/module.h>
#include <xefis/core/module_io.h>
#include <xefis/core/xefis.h>


class Latency:
	public QObject,
	public xf::Module<>
{
	Q_OBJECT

  private:
	static constexpr char kLoggerScope[] = "mod::Latency";

  public:
	// Ctor
	explicit
	Latency (xf::Accounting&, xf::Logger const&, std::string_view const& instance = {});

  private slots:
	/**
	 * Log latencies on module log.
	 */
	void
	log_latency();

  private:
	xf::Logger		_logger;
	xf::Accounting&	_accounting;
	QTimer*			_log_timer;
};

#endif
