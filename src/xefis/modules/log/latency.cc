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

// Lib:
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/accounting.h>

// Local:
#include "latency.h"


Latency::Latency (v2::Accounting& accounting, std::string const& instance):
	Module (instance),
	_accounting (accounting)
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
	v2::Accounting::StatsSet const& event_latency = _accounting.event_latency_stats();

	log() << boost::format ("%-53s min      avg      max") % "--- Latency information ---" << std::endl;
	log() << boost::format ("<%-51s> %0.6lf %.06lf %.06lf")
		% "event handling latency"
		% static_cast<double> (event_latency.select (v2::Accounting::Timespan::Last100Samples).minimum().quantity<Second>())
		% static_cast<double> (event_latency.select (v2::Accounting::Timespan::Last100Samples).average().quantity<Second>())
		% static_cast<double> (event_latency.select (v2::Accounting::Timespan::Last100Samples).maximum().quantity<Second>())
		<< std::endl;

	// Get module stats, sort by average latency and log.

	typedef v2::Accounting::ModuleStats ModuleStats;

	ModuleStats const& mstats = _accounting.module_stats();
	std::vector<ModuleStats::const_iterator> ordered_modules;

	for (ModuleStats::const_iterator m = mstats.begin(); m != mstats.end(); ++m)
		ordered_modules.push_back (m);

	auto order_by_average = [](ModuleStats::const_iterator a, ModuleStats::const_iterator b)
	{
		return
			a->second.select (v2::Accounting::Timespan::Last1000Samples).average() >
			b->second.select (v2::Accounting::Timespan::Last1000Samples).average();
	};

	std::sort (ordered_modules.begin(), ordered_modules.end(), order_by_average);

	for (auto m: ordered_modules)
	{
		log() << boost::format ("[%-30s] %.06lf %.06lf %.06lf")
			% identifier (m->first)
			% static_cast<double> (m->second.select (v2::Accounting::Timespan::Last100Samples).minimum().quantity<Second>())
			% static_cast<double> (m->second.select (v2::Accounting::Timespan::Last100Samples).average().quantity<Second>())
			% static_cast<double> (m->second.select (v2::Accounting::Timespan::Last100Samples).maximum().quantity<Second>())
			<< std::endl;
	}
}

