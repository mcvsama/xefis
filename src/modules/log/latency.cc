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
#include <xefis/core/accounting.h>

// Local:
#include "latency.h"


XEFIS_REGISTER_MODULE_CLASS ("log/latency", Latency);


Latency::Latency (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	_log_timer = new QTimer (this);
	_log_timer->setInterval (1000);
	_log_timer->setSingleShot (false);
	QObject::connect (_log_timer, SIGNAL (timeout()), this, SLOT (log_latency()));
	_log_timer->start();
}


void
Latency::log_latency()
{
	Xefis::Accounting::StatsSet const& event_latency = accounting()->event_latency_stats();
	xdebug ("%-53s min      avg      max\n", "--- Latency information ---");
	xdebug ("<%-51s> %0.6lf %.06lf %.06lf\n",
			"event handling latency",
			(double)event_latency.select (Xefis::Accounting::Timespan::Last100Samples).minimum().s(),
			(double)event_latency.select (Xefis::Accounting::Timespan::Last100Samples).average().s(),
			(double)event_latency.select (Xefis::Accounting::Timespan::Last100Samples).maximum().s());

	// Get module stats, sort by average latency and log.

	typedef Xefis::Accounting::ModuleStats ModuleStats;

	ModuleStats const& ms = accounting()->module_stats();
	std::vector<ModuleStats::const_iterator> ordered_modules;

	for (ModuleStats::const_iterator m = ms.begin(); m != ms.end(); ++m)
		ordered_modules.push_back (m);

	auto order_by_average = [](ModuleStats::const_iterator a, ModuleStats::const_iterator b)
	{
		return
			a->second.select (Xefis::Accounting::Timespan::Last1000Samples).average() >
			b->second.select (Xefis::Accounting::Timespan::Last1000Samples).average();
	};

	std::sort (ordered_modules.begin(), ordered_modules.end(), order_by_average);

	for (auto m: ordered_modules)
	{
		xdebug ("[%-30s#%-20s] %.06lf %.06lf %.06lf\n",
				m->first.name().c_str(), m->first.instance().c_str(),
				static_cast<double> (m->second.select (Xefis::Accounting::Timespan::Last100Samples).minimum().s()),
				static_cast<double> (m->second.select (Xefis::Accounting::Timespan::Last100Samples).average().s()),
				static_cast<double> (m->second.select (Xefis::Accounting::Timespan::Last100Samples).maximum().s()));
	}
}

