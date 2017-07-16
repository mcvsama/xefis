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

#ifndef XEFIS__CORE__ACCOUNTING_H__INCLUDED
#define XEFIS__CORE__ACCOUNTING_H__INCLUDED

// Standard:
#include <cstddef>

// Boost:
#include <boost/circular_buffer.hpp>

// Qt:
#include <QtCore/QTimer>
#include <QtCore/QObject>
#include <QtCore/QEvent>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module.h>
#include <xefis/utility/time_helper.h>


namespace xf {

class Accounting: public QObject
{
	Q_OBJECT

	typedef boost::circular_buffer<Time> LatencySamples;

  private:
	class LatencyCheckEvent: public QEvent
	{
	  public:
		// Ctor
		LatencyCheckEvent();

		/**
		 * Return the time when event was created.
		 */
		Time
		time() const noexcept;

	  private:
		Time _time;
	};

  public:
	enum class Timespan
	{
		Last10Samples,
		Last100Samples,
		Last1000Samples
	};

	class Stats
	{
	  public:
		// Ctor
		explicit
		Stats (LatencySamples::size_type samples);

		/**
		 * Return minimum event handling latency.
		 */
		Time
		minimum() const noexcept;

		/**
		 * Return maximum event handling latency.
		 */
		Time
		maximum() const noexcept;

		/**
		 * Return average event handling latency.
		 */
		Time
		average() const noexcept;

		/**
		 * Add new sample to the stats.
		 */
		void
		new_sample (Time sample);

	  private:
		LatencySamples	_samples;
		mutable Time	_minimum;
		mutable Time	_maximum;
		mutable Time	_average;
		mutable bool	_outdated_minimum = true;
		mutable bool	_outdated_maximum = true;
		mutable bool	_outdated_average = true;
	};

	class StatsSet
	{
	  public:
		Stats const&
		select (Timespan timespan) const noexcept;

	  public:
		Stats	e1	= Stats (10);
		Stats	e2	= Stats (100);
		Stats	e3	= Stats (1000);
	};

	typedef std::map<v1::Module::Pointer, StatsSet> ModuleStats;

  public:
	// Ctor
	Accounting();

	// Dtor
	~Accounting();

	/**
	 * Return latency stats.
	 */
	StatsSet const&
	event_latency_stats() const noexcept;

	/**
	 * Return reference to the ModuleStats structure.
	 * Allows iterating over all accounted modules.
	 */
	ModuleStats const&
	module_stats() const noexcept;

	/**
	 * Return latency stats for given module and timespan.
	 * \throw	ModuleNotFoundException if module can't be found.
	 */
	Stats const&
	module_stats (v1::Module::Pointer, Timespan) const;

	/**
	 * Add module accounting stats (usually called by the ModuleManager
	 * which tracks how much time each module consumes on data_updated().
	 */
	void
	add_module_stats (v1::Module::Pointer, si::Time dt);

  private slots:
	/**
	 * Check and account Qt event loop latency.
	 */
	void
	latency_check();

  private:
	// QObject API
	void
	customEvent (QEvent*) override;

  private:
	Logger		_logger;
	QTimer*		_latency_check_timer;
	StatsSet	_latency_stats;
	ModuleStats	_module_stats;
};


inline
Accounting::LatencyCheckEvent::LatencyCheckEvent():
	QEvent (QEvent::User),
	_time (TimeHelper::now())
{ }


inline Time
Accounting::LatencyCheckEvent::time() const noexcept
{
	return _time;
}


inline Accounting::Stats const&
Accounting::StatsSet::select (Timespan timespan) const noexcept
{
	switch (timespan)
	{
		case Timespan::Last10Samples:
			return e1;
		case Timespan::Last100Samples:
			return e2;
		case Timespan::Last1000Samples:
			return e3;
	}
	return e1;
}


inline Accounting::StatsSet const&
Accounting::event_latency_stats() const noexcept
{
	return _latency_stats;
}


inline Accounting::ModuleStats const&
Accounting::module_stats() const noexcept
{
	return _module_stats;
}

} // namespace xf

#endif

