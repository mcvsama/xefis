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
#include <memory>
#include <type_traits>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/cycle.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/logger.h>


class QWidget;

namespace v2 {
using namespace xf; // XXX

class BasicSetting;
class ModuleIO;


/**
 * A "function" that takes input data in form of input properties, and computes result
 * in form of output properties. Implemented as a class since some modules will have to
 * store some sort of state.
 *
 * Public method that computes the result is fetch_and_process(). It calls implementation-defined
 * process().
 */
class BasicModule: private Noncopyable
{
  public:
	/**
	 * A set of methods for the processing loop to use on the module.
	 */
	class ProcessingLoopAPI
	{
	  public:
		// Ctor
		explicit
		ProcessingLoopAPI (BasicModule*);

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
		BasicModule* _module;
	};

	/**
	 * Defines method for accessing configuration widget if a module decides to implement one.
	 * This class should be inherited by the same class that inherits the BasicModule class.
	 */
	class HasConfiguratorWidget
	{
	  public:
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
	BasicModule (std::unique_ptr<ModuleIO>, std::string const& instance = {});

	// Dtor
	virtual
	~BasicModule() = default;

	/**
	 * Return module instance name.
	 */
	std::string const&
	instance() const noexcept;

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
	std::optional<Logger> mutable	_logger;
	std::unique_ptr<ModuleIO>		_io;
};


template<class IO = ModuleIO>
	class Module: public BasicModule
	{
	  public:
		/**
		 * Ctor
		 * Version for modules that do have their own IO class.
		 */
		template<class = std::enable_if_t<!std::is_same_v<IO, ModuleIO>>>
			explicit
			Module (std::unique_ptr<IO> io, std::string const& instance = {});

		/**
		 * Ctor
		 * Version for modules that do not have any IO class.
		 */
		template<class = std::enable_if_t<std::is_same_v<IO, ModuleIO>>>
			explicit
			Module (std::string const& instance = {});

	  protected:
		IO& io;
	};


inline
BasicModule::ProcessingLoopAPI::ProcessingLoopAPI (BasicModule* module):
	_module (module)
{ }


inline void
BasicModule::ProcessingLoopAPI::reset_cache()
{
	_module->_cached = false;
}


inline std::string const&
BasicModule::instance() const noexcept
{
	return _instance;
}


inline ModuleIO*
BasicModule::io_base() const noexcept
{
	return _io.get();
}


inline Logger const&
BasicModule::log() const
{
	if (!_logger)
	{
		_logger = Logger();
		_logger->set_prefix ((boost::format ("[%-30s#%-20s]") % demangle (typeid (*this).name()) % _instance).str());
	}

	return *_logger;
}


template<class IO>
	template<class>
		inline
		Module<IO>::Module (std::unique_ptr<IO> module_io, std::string const& instance):
			BasicModule (std::move (module_io), instance),
			io (static_cast<IO&> (*io_base()))
		{ }


template<class IO>
	template<class>
		inline
		Module<IO>::Module (std::string const& instance):
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
identifier (BasicModule&);

/**
 * Same as identifier (BasicModule&).
 */
std::string
identifier (BasicModule*);

} // namespace v2

#endif

