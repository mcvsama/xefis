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
#include <algorithm>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module_manager.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "accounting.h"


namespace xf {

Accounting::Stats::Stats (LatencySamples::size_type samples):
	_samples (samples, 0_s)
{ }


inline void
Accounting::Stats::new_sample (Time sample)
{
	_outdated_minimum = true;
	_outdated_maximum = true;
	_outdated_average = true;
	_samples.push_back (sample);
}


Time
Accounting::Stats::minimum() const noexcept
{
	if (_outdated_minimum)
	{
		_minimum = *std::min_element (_samples.begin(), _samples.end());
		_outdated_minimum = false;
	}

	return _minimum;
}


Time
Accounting::Stats::maximum() const noexcept
{
	if (_outdated_maximum)
	{
		_maximum = *std::max_element (_samples.begin(), _samples.end());
		_outdated_maximum = false;
	}

	return _maximum;
}


Time
Accounting::Stats::average() const noexcept
{
	if (_outdated_average)
	{
		_average = 0_s;
		for (Time t: _samples)
			_average += t;
		_average /= _samples.size();
		_outdated_average = false;
	}

	return _average;
}


Accounting::Accounting()
{
	_logger.set_prefix ("<accounting>");
	_logger << "Creating Accounting" << std::endl;
	_latency_check_timer = new QTimer (this);
	_latency_check_timer->setSingleShot (false);
	_latency_check_timer->setInterval (10);
	QObject::connect (_latency_check_timer, SIGNAL (timeout()), this, SLOT (latency_check()));
	_latency_check_timer->start();
}


Accounting::~Accounting()
{
	_logger << "Destroying Accounting" << std::endl;
}


Accounting::Stats const&
Accounting::module_stats (v1::Module::Pointer modptr, Timespan timespan) const
{
	ModuleStats::const_iterator ms = _module_stats.find (modptr);
	if (ms != _module_stats.end())
		return ms->second.select (timespan);
	throw v1::ModuleNotFoundException (QString ("stats for module '%1', instance '%2' can't be found").arg (modptr.name().c_str()).arg (modptr.instance().c_str()).toStdString());
}


void
Accounting::add_module_stats (v1::Module::Pointer modptr, Time dt)
{
	StatsSet& ss = _module_stats[modptr];
	for (Stats* s: { &ss.e1, &ss.e2, &ss.e3 })
		s->new_sample (dt);
}


void
Accounting::latency_check()
{
	QApplication::postEvent (this, new LatencyCheckEvent());
}


void
Accounting::customEvent (QEvent* event)
{
	Time now = TimeHelper::now();
	LatencyCheckEvent* lce = dynamic_cast<LatencyCheckEvent*> (event);
	if (lce)
	{
		Time dt = now - lce->time();
		for (Stats* s: { &_latency_stats.e1, &_latency_stats.e2, &_latency_stats.e3 })
			s->new_sample (dt);
	}
}

} // namespace xf

