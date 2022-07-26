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

#ifndef XEFIS__CORE__PROCESSING_LOOP_H__INCLUDED
#define XEFIS__CORE__PROCESSING_LOOP_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/module_out.h>

// Neutrino:
#include <neutrino/logger.h>
#include <neutrino/noncopyable.h>
#include <neutrino/sequence.h>
#include <neutrino/time.h>
#include <neutrino/tracker.h>

// Qt:
#include <QTimer>

// Standard:
#include <cstddef>


namespace xf {

class Machine;
class Xefis;


class ProcessingLoopIO: public Module
{
  public:
	ModuleOut<si::Frequency>	actual_frequency	{ this, "actual_frequency" };
	ModuleOut<si::Time>			latency				{ this, "latency" };

  public:
	using xf::Module::Module;
};


/**
 * A loop that periodically goes through all modules and calls process() method.
 */
class ProcessingLoop:
	public QObject,
	public ProcessingLoopIO,
	public LoggerTagProvider
{
	Q_OBJECT

	static constexpr std::size_t	kMaxProcessingTimesBackLog	= 1000;
	static constexpr float			kLatencyFactorLogThreshold	= 2.0f;

  public:
	class ModuleDetails
	{
	  public:
		// Ctor
		explicit
		ModuleDetails (Module&);

		Module&
		module() noexcept;

		Module const&
		module() const noexcept;

	  private:
		Module* _module;
	};

	using ModuleDetailsList = std::vector<ModuleDetails>;

  public:
	// Ctor
	explicit
	ProcessingLoop (Machine& machine, std::string_view const& instance, si::Frequency loop_frequency, Logger const&);

	// Dtor
	virtual
	~ProcessingLoop();

	template<class Compatible>
		void
		register_module (Registrant<Compatible>&);

	/**
	 * Return the machine object to which this ProcessingLoop belongs.
	 */
	[[nodiscard]]
	Machine&
	machine() const noexcept;

	/**
	 * Return main Xefis object.
	 */
	[[nodiscard]]
	Xefis&
	xefis() const noexcept;

	/**
	 * Start looping.
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
	period();

	/**
	 * A sequence of modules loaded into this processing loop.
	 */
	[[nodiscard]]
	Sequence<ModuleDetailsList::iterator>
	module_details_list() noexcept;

	/**
	 * A sequence of modules loaded into this processing loop.
	 */
	[[nodiscard]]
	Sequence<ModuleDetailsList::const_iterator>
	module_details_list() const noexcept;

	/**
	 * Processing times buffer.
	 */
	[[nodiscard]]
	boost::circular_buffer<si::Time> const&
	communication_times() const noexcept;

	/**
	 * Processing times buffer.
	 */
	[[nodiscard]]
	boost::circular_buffer<si::Time> const&
	processing_times() const noexcept;

	/**
	 * Processing latencies buffer.
	 */
	[[nodiscard]]
	boost::circular_buffer<si::Time> const&
	processing_latencies() const noexcept;

  protected:
	/**
	 * Execute single loop cycle.
	 */
	virtual void
	execute_cycle();

	// LoggerTagProvider API
	std::optional<std::string>
	logger_tag() const override;

  private:
	ProcessingLoopIO&					_io						{ *this };
	Machine&							_machine;
	Xefis&								_xefis;
	QTimer*								_loop_timer;
	si::Time							_loop_period;
	std::optional<Timestamp>			_previous_timestamp;
	std::vector<Module*>				_uninitialized_modules;
	std::optional<Cycle>				_current_cycle;
	Tracker<Module>						_modules_tracker;
	ModuleDetailsList					_module_details_list;
	boost::circular_buffer<si::Time>	_communication_times	{ kMaxProcessingTimesBackLog };
	boost::circular_buffer<si::Time>	_processing_times		{ kMaxProcessingTimesBackLog };
	boost::circular_buffer<si::Time>	_processing_latencies	{ kMaxProcessingTimesBackLog };
	Cycle::Number						_next_cycle_number		{ 1 };
	Logger								_logger;
};


template<class Compatible>
	inline void
	ProcessingLoop::register_module (Registrant<Compatible>& registrant)
	{
		_modules_tracker.register_object (registrant);
		_module_details_list.emplace_back (*registrant);
		_uninitialized_modules.push_back (&*registrant);
	}


inline
ProcessingLoop::ModuleDetails::ModuleDetails (Module& module):
	_module (&module)
{ }


inline Module&
ProcessingLoop::ModuleDetails::module() noexcept
{
	return *_module;
}


inline Module const&
ProcessingLoop::ModuleDetails::module() const noexcept
{
	return *_module;
}


inline Machine&
ProcessingLoop::machine() const noexcept
{
	return _machine;
}


inline Xefis&
ProcessingLoop::xefis() const noexcept
{
	return _xefis;
}


inline Cycle const*
ProcessingLoop::current_cycle() const
{
	return _current_cycle
		? &_current_cycle.value()
		: nullptr;
}


inline si::Time
ProcessingLoop::period()
{
	return _loop_period;
}


inline auto
ProcessingLoop::module_details_list() noexcept -> Sequence<ModuleDetailsList::iterator>
{
	return { _module_details_list.begin(), _module_details_list.end() };
}


inline auto
ProcessingLoop::module_details_list() const noexcept -> Sequence<ModuleDetailsList::const_iterator>
{
	return { _module_details_list.cbegin(), _module_details_list.cend() };
}


inline boost::circular_buffer<si::Time> const&
ProcessingLoop::communication_times() const noexcept
{
	return _communication_times;
}


inline boost::circular_buffer<si::Time> const&
ProcessingLoop::processing_times() const noexcept
{
	return _processing_times;
}


inline boost::circular_buffer<si::Time> const&
ProcessingLoop::processing_latencies() const noexcept
{
	return _processing_latencies;
}

} // namespace xf

#endif

