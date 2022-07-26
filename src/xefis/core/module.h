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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/cycle.h>
#include <xefis/utility/named_instance.h>

// Neutrino:
#include <neutrino/noncopyable.h>
#include <neutrino/sequence.h>

// Lib:
#include <boost/circular_buffer.hpp>

// Standard:
#include <cstddef>
#include <vector>
#include <exception>
#include <optional>
#include <memory>
#include <type_traits>


class QWidget;

namespace xf {

class BasicSetting;
class BasicModuleIn;
class BasicModuleOut;


/**
 * Exception thrown when some settings in a module have not been initialized as required.
 */
class UninitializedSettings: public FastException
{
  public:
	// Ctor
	explicit
	UninitializedSettings (std::vector<BasicSetting*>);

  private:
	/**
	 * Create a message for the exception.
	 */
	std::string
	make_message (std::vector<BasicSetting*>);
};


/**
 * A "function" that takes input data in form of input sockets, and computes result
 * in form of output sockets. Implemented as a class since some modules will have to
 * store some sort of state.
 *
 * Public method that computes the result is fetch_and_process(). It calls implementation-defined
 * process().
 */
class Module:
	public NamedInstance,
	private Noncopyable
{
	static constexpr std::size_t kMaxProcessingTimesBackLog = 1000;

  public:
	/**
	 * A set of methods for module socket to use on the module.
	 */
	class ModuleSocketAPI
	{
	  public:
		// Ctor
		explicit
		ModuleSocketAPI (Module& module):
			_module (module)
		{ }

		// Dtor
		virtual
		~ModuleSocketAPI() = default;

		/**
		 * Set reference to the module object.
		 */
		void
		set_module (Module&);

		/**
		 * Iterate through registered settings and check that ones without default value have been initialized by user.
		 * If uninitialized settings are found, UninitializedSettings is thrown.
		 * Also call virtual Module::verify_settings().
		 */
		virtual void
		verify_settings();

		/**
		 * Register setting.
		 */
		void
		register_setting (BasicSetting&);

		/**
		 * Register an input socket with this module.
		 */
		void
		register_input_socket (BasicModuleIn&);

		/**
		 * Unregister an input socket.
		 */
		void
		unregister_input_socket (BasicModuleIn&);

		/**
		 * Register an output socket with this module.
		 */
		void
		register_output_socket (BasicModuleOut&);

		/**
		 * Unregister an output socket.
		 */
		void
		unregister_output_socket (BasicModuleOut&);

		/**
		 * Return registered settings.
		 */
		Sequence<std::vector<BasicSetting*>::const_iterator>
		settings() const noexcept;

		/**
		 * Return registered input sockets.
		 */
		Sequence<std::vector<BasicModuleIn*>::const_iterator>
		input_sockets() const noexcept;

		/**
		 * Return registered output sockets.
		 */
		Sequence<std::vector<BasicModuleOut*>::const_iterator>
		output_sockets() const noexcept;

	  private:
		Module& _module;
	};

	/**
	 * A set of methods for the processing loop to use on the module.
	 */
	class ProcessingLoopAPI
	{
	  public:
		// Ctor
		explicit
		ProcessingLoopAPI (Module&);

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
		 * Request all connected input sockets to be computed, and then
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
		Module& _module;
	};

	/**
	 * Accesses accounting data (time spent on processing, etc.
	 */
	class AccountingAPI
	{
	  public:
		// Ctor
		explicit
		AccountingAPI (Module&);

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
		Module& _module;
	};

	/**
	 * Defines method for accessing configuration widget if a module decides to implement one.
	 * This class should be inherited by the same class that inherits the Module class.
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
	 * \param	instance
	 *			Instance name for GUI identification and debugging purposes.
	 */
	explicit
	Module (std::string_view const& instance = {});

	// Dtor
	virtual
	~Module();

	/**
	 * Initialize before starting the loop.
	 * Default implementation does nothing.
	 */
	virtual void
	initialize();

	/**
	 * User-provided settings verification procedure.
	 */
	virtual void
	verify_settings()
	{ }

  protected:
	/**
	 * Communicate with sensors/actuators to send/receive processing data and results.
	 * This method should be as fast as possible, actual data processing must happen
	 * in process().
	 */
	virtual void
	communicate (Cycle const& cycle);

	/**
	 * Compute output sockets.
	 * Default implementation does nothing.
	 */
	virtual void
	process (Cycle const& cycle);

	/**
	 * Called when exception is caught from the process() method.
	 * Default implementation logs the exception and sets all output sockets to nil.
	 */
	virtual void
	rescue (Cycle const&, std::exception_ptr);

	/**
	 * Enable/disable option to set all output sockets to xf::nil when
	 * exception occurs within the process(Cycle) method.
	 * This happens after calling the rescue() method.
	 *
	 * By default it's enabled.
	 */
	void
	set_nil_on_exception (bool enable) noexcept;

  private:
	std::vector<BasicSetting*>			_registered_settings;
	std::vector<BasicModuleIn*>			_registered_input_sockets;
	std::vector<BasicModuleOut*>		_registered_output_sockets;
	bool								_did_not_communicate: 1		{ false };
	bool								_did_not_process: 1			{ false };
	bool								_cached: 1					{ false };
	bool								_set_nil_on_exception: 1	{ true };
	boost::circular_buffer<si::Time>	_communication_times		{ kMaxProcessingTimesBackLog };
	boost::circular_buffer<si::Time>	_processing_times			{ kMaxProcessingTimesBackLog };
	si::Time							_cycle_time					{ 0_s };
};


inline
Module::ProcessingLoopAPI::ProcessingLoopAPI (Module& module):
	_module (module)
{ }


inline bool
Module::ProcessingLoopAPI::implements_communicate_method() const noexcept
{
	return !_module._did_not_communicate;
}


inline bool
Module::ProcessingLoopAPI::implements_process_method() const noexcept
{
	return !_module._did_not_process;
}


inline void
Module::ProcessingLoopAPI::reset_cache()
{
	_module._cached = false;
}


inline
Module::AccountingAPI::AccountingAPI (Module& module):
	_module (module)
{ }


inline si::Time
Module::AccountingAPI::cycle_time() const noexcept
{
	return _module._cycle_time;
}


inline void
Module::AccountingAPI::set_cycle_time (si::Time cycle_time)
{
	_module._cycle_time = cycle_time;
}


inline void
Module::AccountingAPI::add_communication_time (si::Time t)
{
	_module._communication_times.push_back (t);
}


inline void
Module::AccountingAPI::add_processing_time (si::Time t)
{
	_module._processing_times.push_back (t);
}


inline boost::circular_buffer<si::Time> const&
Module::AccountingAPI::communication_times() const noexcept
{
	return _module._communication_times;
}


inline boost::circular_buffer<si::Time> const&
Module::AccountingAPI::processing_times() const noexcept
{
	return _module._processing_times;
}


inline void
Module::set_nil_on_exception (bool enable) noexcept
{
	_set_nil_on_exception = enable;
}


/*
 * Global functions
 */


/**
 * Return string identifying module and its instance.
 */
std::string
identifier (Module const&);

/**
 * Same as identifier (Module&).
 */
std::string
identifier (Module const*);

} // namespace xf

#endif

