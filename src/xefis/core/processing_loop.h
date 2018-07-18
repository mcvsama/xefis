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
#include <xefis/core/property.h>
#include <xefis/utility/sequence.h>
#include <xefis/utility/time.h>


namespace xf {

class Machine;
class Xefis;
class Logger;


/**
 * A loop that periodically goes through all modules and calls process() method.
 */
class ProcessingLoop: public QObject
{
	Q_OBJECT

  public:
	class ModuleDetails
	{
	  public:
		// Ctor
		explicit
		ModuleDetails (Unique<BasicModule>);

		BasicModule&
		module() noexcept;

		BasicModule const&
		module() const noexcept;

	  public:
		// TODO more time accounting
		si::Time			last_processing_time	{ 0_ms };

	  private:
		Unique<BasicModule>	_module;
	};

	using ModulesContainer = std::vector<ModuleDetails>;

  public:
	//TODO PropertyOut<si::Frequency>	_actual_frequency	{ "/system/processing-loop/x/actual-frequency" };
	//TODO PropertyOut<si::Time>		_latency			{ "/system/processing-loop/x/latency" };

  public:
	// Ctor
	explicit
	ProcessingLoop (Machine* machine, std::string const& name, Frequency loop_frequency);

	// Dtor
	virtual
	~ProcessingLoop();

	/**
	 * Load module by name and return a pointer.
	 * Module is owned by Machine object.
	 */
	template<class pModule, class ...Arg>
		pModule*
		load_module (Arg&& ...args);

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
	Sequence<ModulesContainer::iterator>
	module_details_list() noexcept;

	/**
	 * A sequence of modules loaded into this processing loop.
	 */
	Sequence<ModulesContainer::const_iterator>
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
	std::unique_ptr<Logger>		_logger;
	ModulesContainer			_modules;
};


template<class pModule, class ...Arg>
	inline pModule*
	ProcessingLoop::load_module (Arg&& ...args)
	{
		auto module = std::make_unique<pModule> (std::forward<Arg> (args)...);
		auto module_raw_ptr = module.get();

		_modules.push_back (ModuleDetails (std::move (module)));
		_uninitialized_modules.push_back (module_raw_ptr);

		return module_raw_ptr;
	}


inline
ProcessingLoop::ModuleDetails::ModuleDetails (Unique<BasicModule> module):
	_module (std::move (module))
{ }


inline BasicModule&
ProcessingLoop::ModuleDetails::module() noexcept
{
	return *_module.get();
}


inline BasicModule const&
ProcessingLoop::ModuleDetails::module() const noexcept
{
	return *_module.get();
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
ProcessingLoop::module_details_list() noexcept -> Sequence<ModulesContainer::iterator>
{
	return { _modules.begin(), _modules.end() };
}


inline auto
ProcessingLoop::module_details_list() const noexcept -> Sequence<ModulesContainer::const_iterator>
{
	return { _modules.cbegin(), _modules.cend() };
}

} // namespace xf

#endif

