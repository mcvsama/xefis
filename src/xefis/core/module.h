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

#ifndef XEFIS__CORE__MODULE_H__INCLUDED
#define XEFIS__CORE__MODULE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>
#include <exception>
#include <optional>
#include <memory>
#include <type_traits>

// Lib:
#include <boost/circular_buffer.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/cycle.h>
#include <xefis/core/module_io.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/named_instance.h>


class QWidget;

namespace xf {

class BasicSetting;


/**
 * Type-tag used to indicate that Module<> doesn't have any ModuleIO class.
 */
class NoModuleIO: public ModuleIO
{ };


/**
 * A "function" that takes input data in form of input properties, and computes result
 * in form of output properties. Implemented as a class since some modules will have to
 * store some sort of state.
 *
 * Public method that computes the result is fetch_and_process(). It calls implementation-defined
 * process().
 */
class BasicModule:
	public NamedInstance,
	private Noncopyable
{
	static constexpr std::size_t kMaxProcessingTimesBackLog = 1000;

  public:
	/**
	 * A set of methods for the processing loop to use on the module.
	 */
	class ProcessingLoopAPI
	{
	  public:
		// Ctor
		explicit
		ProcessingLoopAPI (BasicModule&);

		/**
		 * True if module implements its own communicate() method.
		 * Result is valid after the execution of first cycle, otherwise true is returned.
		 */
		[[nodiscard]]
		bool
		implements_communicate_method() const noexcept;

		/**
		 * True if module implements its own process() method.
		 * Result is valid after the execution of first cycle, otherwise true is returned.
		 */
		[[nodiscard]]
		bool
		implements_process_method() const noexcept;

		/**
		 * Request all modules to communicate with external systems (hardware systems)
		 * and store a snapshot of parameters that will be used in the process() method.
		 */
		void
		communicate (Cycle const&);

		/**
		 * Request all connected input properties to be computed, and then
		 * call the process() method. It will compute results only once, until
		 * reset_cache() is called.
		 */
		void
		fetch_and_process (Cycle const&);

		/**
		 * Delete cached result of fetch_and_process().
		 */
		void
		reset_cache();

	  private:
		/**
		 * Print current exception information.
		 */
		void
		handle_exception (Cycle const&, std::string_view const& context_info);

	  private:
		BasicModule& _module;
	};

	/**
	 * Accesses accounting data (time spent on processing, etc.
	 */
	class AccountingAPI
	{
	  public:
		// Ctor
		explicit
		AccountingAPI (BasicModule&);

		/**
		 * Cycle time of the ProcessingLoop that this module is being processed in.
		 */
		[[nodiscard]]
		si::Time
		cycle_time() const noexcept;

		/**
		 * Set cycle time of he ProcessingLoop that this module is being processed in.
		 */
		void
		set_cycle_time (si::Time);

		/**
		 * Add new measured communication time (time spent in the communicate() method).
		 */
		void
		add_communication_time (si::Time);

		/**
		 * Add new measured processing time (time spent in the process() method).
		 */
		void
		add_processing_time (si::Time);

		/**
		 * Communication times buffer.
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

	  private:
		BasicModule& _module;
	};

	/**
	 * Defines method for accessing configuration widget if a module decides to implement one.
	 * This class should be inherited by the same class that inherits the BasicModule class.
	 */
	class HasConfiguratorWidget
	{
	  public:
		// Dtor
		virtual
		~HasConfiguratorWidget() = default;

		virtual QWidget*
		configurator_widget() = 0;
	};

  public:
	/**
	 * Ctor
	 *
	 * \param	module_io
	 *			Object that storess all Settings, PropertyIns and PropertyOuts.
	 * \param	instance
	 *			Instance name for GUI identification and debugging purposes.
	 */
	explicit
	BasicModule (std::unique_ptr<ModuleIO>, std::string_view const& instance = {});

	// Dtor
	virtual
	~BasicModule() = default;

	/**
	 * Return the IO object of this module.
	 */
	ModuleIO*
	io_base() const noexcept;

	/**
	 * Initialize before starting the loop.
	 * Default implementation does nothing.
	 */
	virtual void
	initialize();

  protected:
	/**
	 * Communicate with sensors/actuators to send/receive processing data and results.
	 * This method should be as fast as possible, actual data processing must happen
	 * in process().
	 */
	virtual void
	communicate (Cycle const& cycle);

	/**
	 * Compute output properties.
	 * Default implementation does nothing.
	 */
	virtual void
	process (Cycle const& cycle);

	/**
	 * Called when exception is caught from the process() method.
	 * Default implementation logs the exception and sets all output properties to nil.
	 */
	virtual void
	rescue (Cycle const&, std::exception_ptr);

	/**
	 * Enable/disable option to set all output properties to xf::nil when
	 * exception occurs within the process(Cycle) method.
	 * This happens after calling the rescue() method.
	 *
	 * By default it's enabled.
	 */
	void
	set_nil_on_exception (bool enable) noexcept;

  private:
	bool								_did_not_communicate	{ false };
	bool								_did_not_process		{ false };
	bool								_cached					{ false };
	bool								_set_nil_on_exception	{ true };
	std::unique_ptr<ModuleIO>			_io;
	boost::circular_buffer<si::Time>	_communication_times	{ kMaxProcessingTimesBackLog };
	boost::circular_buffer<si::Time>	_processing_times		{ kMaxProcessingTimesBackLog };
	si::Time							_cycle_time				{ 0_s };
};


template<class IO = NoModuleIO>
	class Module: public BasicModule
	{
	  public:
		/**
		 * Ctor
		 * Version for modules that do have their own IO class.
		 */
		template<class = std::enable_if_t<!std::is_same_v<IO, NoModuleIO>>>
			explicit
			Module (std::unique_ptr<IO> io, std::string_view const& instance = {});

		/**
		 * Ctor
		 * Version for modules that do not have any IO class.
		 */
		template<class = std::enable_if_t<std::is_same_v<IO, NoModuleIO>>>
			explicit
			Module (std::string_view const& instance = {});

	  protected:
		IO& io;
	};


inline
BasicModule::ProcessingLoopAPI::ProcessingLoopAPI (BasicModule& module):
	_module (module)
{ }


inline bool
BasicModule::ProcessingLoopAPI::implements_communicate_method() const noexcept
{
	return !_module._did_not_communicate;
}


inline bool
BasicModule::ProcessingLoopAPI::implements_process_method() const noexcept
{
	return !_module._did_not_process;
}


inline void
BasicModule::ProcessingLoopAPI::reset_cache()
{
	_module._cached = false;
}


inline
BasicModule::AccountingAPI::AccountingAPI (BasicModule& module):
	_module (module)
{ }


inline si::Time
BasicModule::AccountingAPI::cycle_time() const noexcept
{
	return _module._cycle_time;
}


inline void
BasicModule::AccountingAPI::set_cycle_time (si::Time cycle_time)
{
	_module._cycle_time = cycle_time;
}


inline void
BasicModule::AccountingAPI::add_communication_time (si::Time t)
{
	_module._communication_times.push_back (t);
}


inline void
BasicModule::AccountingAPI::add_processing_time (si::Time t)
{
	_module._processing_times.push_back (t);
}


inline boost::circular_buffer<si::Time> const&
BasicModule::AccountingAPI::communication_times() const noexcept
{
	return _module._communication_times;
}


inline boost::circular_buffer<si::Time> const&
BasicModule::AccountingAPI::processing_times() const noexcept
{
	return _module._processing_times;
}


inline ModuleIO*
BasicModule::io_base() const noexcept
{
	return _io.get();
}


inline void
BasicModule::set_nil_on_exception (bool enable) noexcept
{
	_set_nil_on_exception = enable;
}


template<class IO>
	template<class>
		inline
		Module<IO>::Module (std::unique_ptr<IO> module_io, std::string_view const& instance):
			BasicModule (std::move (module_io), instance),
			io (static_cast<IO&> (*io_base()))
		{ }


template<class IO>
	template<class>
		inline
		Module<IO>::Module (std::string_view const& instance):
			BasicModule (std::make_unique<IO>(), instance),
			io (static_cast<IO&> (*io_base()))
		{ }


/*
 * Global functions
 */


/**
 * Return string identifying module and its instance.
 */
std::string
identifier (BasicModule const&);

/**
 * Same as identifier (BasicModule&).
 */
std::string
identifier (BasicModule const*);

} // namespace xf

#endif

