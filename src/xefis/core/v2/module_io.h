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

#ifndef XEFIS__CORE__V2__MODULE_IO_H__INCLUDED
#define XEFIS__CORE__V2__MODULE_IO_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/cycle.h>
#include <xefis/core/v2/module.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/logger.h>


class QWidget;

namespace v2 {
using namespace xf; // XXX

class BasicModule;
class BasicSetting;
class BasicPropertyIn;
class BasicPropertyOut;


namespace module_io {

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
 * Exception thrown when there's general logic error in configuration.
 */
class InvalidConfig: public Exception
{
  public:
	// Ctor
	explicit
	InvalidConfig (std::string const& message):
		Exception ("logic error in ModuleIO configuration: " + message)
	{ }
};

} // namespace module_io


class ModuleIO
{
	friend class BasicModule;
	friend class BasicModule::ProcessingLoopAPI;

  public:
	/**
	 * A set of methods for the processing loop to use on the module.
	 */
	class ProcessingLoopAPI
	{
	  public:
		// Ctor
		explicit
		ProcessingLoopAPI (ModuleIO*);

		/**
		 * Set reference to the module object.
		 */
		void
		set_module (BasicModule*);

		/**
		 * Iterate through registered settings and check that ones without default value have been initialized by user.
		 * If uninitialized settings are found, UninitializedSettings is thrown.
		 * Also call virtual ModuleIO::verify_settings().
		 */
		virtual void
		verify_settings();

		/**
		 * Register setting.
		 */
		void
		register_setting (BasicSetting*);

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
		ModuleIO* _io;
	};

  public:
	/**
	 * Return reference to the module that uses this ModuleIO object.
	 */
	BasicModule*
	module() const noexcept;

	/**
	 * User settings verification procedure.
	 */
	virtual void
	verify_settings()
	{ }

  private:
	BasicModule*					_module = nullptr;
	std::vector<BasicSetting*>		_registered_settings;
	std::vector<BasicPropertyIn*>	_registered_input_properties;
	std::vector<BasicPropertyOut*>	_registered_output_properties;
};


inline
ModuleIO::ProcessingLoopAPI::ProcessingLoopAPI (ModuleIO* io):
	_io (io)
{ }


inline void
ModuleIO::ProcessingLoopAPI::set_module (BasicModule* module)
{
	_io->_module = module;
}


inline BasicModule*
ModuleIO::module() const noexcept
{
	return _module;
}


/*
 * Global functions
 */


/**
 * Return string identifying module and its instance, if any module is associated with the ModuleIO object.
 */
std::string
identifier (ModuleIO&);


/**
 * Same as identifier (ModuleIO&).
 */
std::string
identifier (ModuleIO*);

} // namespace v2

#endif

