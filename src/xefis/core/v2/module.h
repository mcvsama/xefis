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

#ifndef XEFIS__CORE__V2__MODULE_H__INCLUDED
#define XEFIS__CORE__V2__MODULE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>
#include <exception>
#include <optional>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/cycle.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/logger.h>


class QWidget;

namespace v2 {
using namespace xf; // XXX

class BasicSetting;
class BasicPropertyIn;
class BasicPropertyOut;


/**
 * Exception object thrown when some settings in a module have not been initialized as required.
 */
class UninitializedSettings: public Exception
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
 * A "function" that takes input data in form of input properties, and computes result
 * in form of output properties. Implemented as a class since some modules will have to
 * store some sort of state.
 *
 * Public method that computes the result is fetch_and_process(). It calls implementation-defined
 * process().
 */
class Module: private Noncopyable
{
  public:
	/**
	 * A set of methods for processing loop to
	 */
	class ProcessingLoopAPI
	{
	  public:
		// Ctor
		explicit
		ProcessingLoopAPI (Module*);

		/**
		 * Iterate through registered settings and check that ones without default value have been initialized by user.
		 * If uninitialized settings are found, UninitializedSettings is thrown.
		 */
		void
		verify_settings();

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

		/**
		 * Register an input property with this module.
		 */
		void
		register_input_property (BasicPropertyIn*);

		/**
		 * Unregister an input property.
		 */
		void
		unregister_input_property (BasicPropertyIn*);

		/**
		 * Register an output property with this module.
		 */
		void
		register_output_property (BasicPropertyOut*);

		/**
		 * Unregister an output property.
		 */
		void
		unregister_output_property (BasicPropertyOut*);

	  private:
		Module* _module;
	};

	/**
	 * Defines method for accessing configuration widget if a module decides to implement one.
	 * This class should be inherited by the same class that inherits the Module class.
	 */
	class HasConfiguratorWidget
	{
	  public:
		virtual QWidget*
		configurator_widget() = 0;
	};

  public:
	// Ctor
	explicit
	Module (std::string const& instance = {});

	// Dtor
	virtual
	~Module() = default;

	/**
	 * Return module instance name.
	 */
	std::string const&
	instance() const noexcept;

	/**
	 * Initialize before starting the loop.
	 * Default implementation does nothing.
	 */
	virtual void
	initialize();

  protected:
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
	rescue (std::exception_ptr);

	/**
	 * Add header with module name to the log stream and return the stream.
	 */
	Logger const&
	log() const;

  private:
	std::string						_instance;
	bool							_cached = false;
	std::vector<BasicSetting*>		_registered_settings;
	std::vector<BasicPropertyIn*>	_registered_input_properties;
	std::vector<BasicPropertyOut*>	_registered_output_properties;
	std::optional<Logger> mutable	_logger;
};


inline
Module::ProcessingLoopAPI::ProcessingLoopAPI (Module* module):
	_module (module)
{ }


inline void
Module::ProcessingLoopAPI::reset_cache()
{
	_module->_cached = false;
}


inline std::string const&
Module::instance() const noexcept
{
	return _instance;
}


inline Logger const&
Module::log() const
{
	if (!_logger)
	{
		_logger = Logger();
		_logger->set_prefix ((boost::format ("[%-30s#%-20s]") % demangle (typeid (*this).name()) % _instance).str());
	}

	return *_logger;
}


/*
 * Global functions
 */


/**
 * Return string identifying module and its instance.
 */
std::string
module_identifier (Module&);

/**
 * Same as module_identifier (Module&).
 */
std::string
module_identifier (Module*);

} // namespace v2

#endif

