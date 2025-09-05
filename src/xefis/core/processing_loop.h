/* vim:ts=4
 *
 * Copyleft 2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__PROCESSING_LOOP_H__INCLUDED
#define XEFIS__CORE__PROCESSING_LOOP_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/module_out.h>

// Neutrino:
#include <neutrino/logger.h>
#include <neutrino/noncopyable.h>
#include <neutrino/time.h>

// Qt:
#include <QTimer>

// Standard:
#include <cstddef>
#include <memory>
#include <optional>
#include <ranges>
#include <variant>
#include <vector>


namespace xf {

// Forwarded for convenience:
class Machine;
class Xefis;


/**
 * A loop that periodically goes through all modules and calls process() method.
 */
class ProcessingLoop:
	public QObject,
	public Module,
	public nu::LoggerTagProvider
{
  private:
	/**
	 * The loop uses its own internal timer for advancing time.
	 */
	struct InternalTimer
	{
		explicit
		InternalTimer (ProcessingLoop&);

		QTimer	timer;
	};

	/**
	 * The loop has to be ticked by external actor using the advance() method.
	 */
	struct ExternalTimer
	{
		si::Time	current_time;
		si::Time	integrated_time;
	};

	using Timer = std::variant<std::monostate, InternalTimer, ExternalTimer>;

  public:
	ModuleOut<si::Frequency>	actual_frequency	{ this, "actual_frequency" };
	ModuleOut<si::Time>			latency				{ this, "latency" };

  private:
	static constexpr std::size_t	kMaxProcessingTimesBackLog	= 1000;
	static constexpr float			kLatencyFactorLogThreshold	= 2.0f;

	using Modules = std::vector<Module*>;

  public:
	// Ctor
	explicit
	ProcessingLoop (std::string_view const instance, si::Frequency loop_frequency, nu::Logger const&);

	explicit
	ProcessingLoop (ProcessingLoop const&) = delete;

	explicit
	ProcessingLoop (ProcessingLoop&&) = delete;

	ProcessingLoop&
	operator= (ProcessingLoop const&) = delete;

	ProcessingLoop&
	operator= (ProcessingLoop&&) = delete;

	/**
	 * Register a module in the processing loop. The module must be destroyed
	 * before loop is destroyed.
	 */
	void
	register_module (Module&);

	/**
	 * Start looping using the internal timer.
	 * On first call, initializes modules that were not initialized yet.
	 */
	void
	start();

	/**
	 * Stop looping.
	 */
	void
	stop();

	/**
	 * Pause/unpause.
	 * Paused loop doesn't execute cycles regardless if internal or external
	 * timer is used.
	 * Loop is unpaused by default.
	 */
	void
	set_paused (bool paused)
		{ _paused = paused; }

	/**
	 * Advances loop time by given amount and executes appropriate number of cycles.
	 * If internal timer was used, it gets stopped and the loop is switched to use
	 * external timer.
	 */
	void
	advance (si::Time);

	/**
	 * Switches to external timer and sets current time.
	 */
	void
	set_external_timer_time (si::Time);

	/**
	 * Return current processing cycle, if called during a processing cycle.
	 * Otherwise return nullptr.
	 */
	[[nodiscard]]
	Cycle const*
	current_cycle() const;

	/**
	 * Processing cycle period.
	 */
	[[nodiscard]]
	si::Time
	period()
		{ return _loop_period; }

	/**
	 * A sequence of modules loaded into this processing loop.
	 */
	[[nodiscard]]
	std::ranges::subrange<Modules::iterator>
	modules() noexcept
		{ return _modules; }

	/**
	 * A sequence of modules loaded into this processing loop.
	 */
	[[nodiscard]]
	std::ranges::subrange<Modules::const_iterator>
	modules() const noexcept
		{ return _modules; }

	/**
	 * Processing times buffer.
	 */
	[[nodiscard]]
	boost::circular_buffer<si::Time> const&
	communication_times() const noexcept
		{ return _communication_times; }

	/**
	 * Processing times buffer.
	 */
	[[nodiscard]]
	boost::circular_buffer<si::Time> const&
	processing_times() const noexcept
		{ return _processing_times; }

	/**
	 * Processing latencies buffer.
	 */
	[[nodiscard]]
	boost::circular_buffer<si::Time> const&
	processing_latencies() const noexcept
		{ return _processing_latencies; }

  protected:
	/**
	 * Initialize not yet initialized modules.
	 */
	void
	init_uninitialized_modules();

	/**
	 * Switches to internal timer.
	 */
	InternalTimer&
	switch_to_internal_timer();

	/**
	 * Switches to external timer.
	 */
	ExternalTimer&
	switch_to_external_timer (si::Time current_time = nu::utc_now());

	/**
	 * Execute single loop cycle assuming that the current time is
	 * given by "now".
	 */
	virtual void
	execute_cycle (si::Time now);

	// LoggerTagProvider API
	std::optional<std::string>
	logger_tag() const override;

  private:
	si::Time							_loop_period;
	Timer								_loop_timer;
	std::optional<nu::Timestamp>		_previous_timestamp;
	std::vector<Module*>				_uninitialized_modules;
	std::optional<Cycle>				_current_cycle;
	Modules								_modules;
	boost::circular_buffer<si::Time>	_communication_times	{ kMaxProcessingTimesBackLog };
	boost::circular_buffer<si::Time>	_processing_times		{ kMaxProcessingTimesBackLog };
	boost::circular_buffer<si::Time>	_processing_latencies	{ kMaxProcessingTimesBackLog };
	Cycle::Number						_next_cycle_number		{ 1 };
	nu::Logger							_logger;
	bool								_paused					{ false };
};


inline void
ProcessingLoop::register_module (Module& module)
{
	_modules.push_back (&module);
	_uninitialized_modules.push_back (&module);
}


inline Cycle const*
ProcessingLoop::current_cycle() const
{
	return _current_cycle
		? &_current_cycle.value()
		: nullptr;
}

} // namespace xf

#endif

