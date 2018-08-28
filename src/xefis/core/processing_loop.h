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

// Standard:
#include <cstddef>

// Qt:
#include <QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/logger.h>
#include <xefis/core/property.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/sequence.h>
#include <xefis/utility/time.h>
#include <xefis/utility/tracker.h>


namespace xf {

class Machine;
class Xefis;
class Logger;


class ProcessingLoopIO: public ModuleIO
{
  private:
	std::string const			_loop_name;

  public:
	PropertyOut<si::Frequency>	actual_frequency	{ this, "/system/processing-loop/" + _loop_name + "/actual_frequency" };
	PropertyOut<si::Time>		latency				{ this, "/system/processing-loop/" + _loop_name + "/latency" };

  public:
	ProcessingLoopIO (std::string const& loop_name):
		_loop_name (loop_name)
	{ }
};


/**
 * A loop that periodically goes through all modules and calls process() method.
 */
class ProcessingLoop:
	public QObject,
	public Module<ProcessingLoopIO>
{
	Q_OBJECT

  public:
	class ModuleDetails
	{
	  public:
		// Ctor
		explicit
		ModuleDetails (BasicModule&);

		BasicModule&
		module() noexcept;

		BasicModule const&
		module() const noexcept;

	  public:
		// TODO more time accounting
		si::Time		last_processing_time	{ 0_ms };

	  private:
		BasicModule*	_module;
	};

	using ModuleDetailsList = std::vector<ModuleDetails>;

  public:
	// Ctor
	explicit
	ProcessingLoop (Machine* machine, std::string const& name, Frequency loop_frequency, Logger const&);

	// Dtor
	virtual
	~ProcessingLoop();

	template<class Compatible>
		void
		register_module (Registrant<Compatible>&);

	/**
	 * Return the machine object to which this ProcessingLoop belongs.
	 */
	Machine*
	machine() const noexcept;

	/**
	 * Return main Xefis object.
	 */
	Xefis*
	xefis() const noexcept;

	/**
	 * Processing loop UI name.
	 */
	std::string const&
	name() const noexcept;

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
	 * Otherwise return empty std::optional.
	 */
	Cycle const*
	current_cycle() const;

	/**
	 * A sequence of modules loaded into this processing loop.
	 */
	Sequence<ModuleDetailsList::iterator>
	module_details_list() noexcept;

	/**
	 * A sequence of modules loaded into this processing loop.
	 */
	Sequence<ModuleDetailsList::const_iterator>
	module_details_list() const noexcept;

  protected:
	/**
	 * Execute single loop cycle.
	 */
	virtual void
	execute_cycle();

  private:
	Machine*					_machine;
	Xefis*						_xefis;
	std::string					_name;
	QTimer*						_loop_timer;
	Time						_loop_period		{ 10_ms };
	std::optional<Timestamp>	_previous_timestamp;
	std::vector<BasicModule*>	_uninitialized_modules;
	std::optional<Cycle>		_current_cycle;
	Cycle::Number				_next_cycle_number	{ 1 };
	Logger						_logger;
	Tracker<BasicModule>		_modules_tracker; // TODO use ModuleDetails as Tracker<> second parameter
	ModuleDetailsList			_module_details_list;
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
ProcessingLoop::ModuleDetails::ModuleDetails (BasicModule& module):
	_module (&module)
{ }


inline BasicModule&
ProcessingLoop::ModuleDetails::module() noexcept
{
	return *_module;
}


inline BasicModule const&
ProcessingLoop::ModuleDetails::module() const noexcept
{
	return *_module;
}


inline Machine*
ProcessingLoop::machine() const noexcept
{
	return _machine;
}


inline Xefis*
ProcessingLoop::xefis() const noexcept
{
	return _xefis;
}


inline std::string const&
ProcessingLoop::name() const noexcept
{
	return _name;
}


inline Cycle const*
ProcessingLoop::current_cycle() const
{
	return _current_cycle
		? &_current_cycle.value()
		: nullptr;
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

} // namespace xf

#endif

