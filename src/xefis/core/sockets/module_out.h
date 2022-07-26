/* vim:ts=4
 *
 * Copyleft 2021  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__SOCKETS__MODULE_OUT_H__INCLUDED
#define XEFIS__CORE__SOCKETS__MODULE_OUT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/assignable_socket.h>
#include <xefis/core/sockets/basic_module_out.h>

// Neutrino:
#include <neutrino/blob.h>

// Standard:
#include <cstddef>
#include <cstdint>


namespace xf {

/**
 * ModuleSocket that acts as an output from the module.
 * Can fetch not from other sockets, but from Module instead (call module's process() to get it
 * to set a value on the Socket).
 */
template<class pValue>
	class ModuleOut final:
		public BasicModuleOut,
		public AssignableSocket<pValue>
	{
	  public:
		typedef pValue Value;

	  public:
		using AssignableSocket<pValue>::operator=;

		/**
		 * Create ModuleOut that's coupled to a Module and set the module as data source.
		 */
		explicit
		ModuleOut (Module* owner_and_data_source, std::string_view const& path);

		// Dtor
		~ModuleOut();

		// BasicSocket API
		void
		do_fetch (Cycle const&) override;

		// BasicModuleSocket API
		void
		deregister() override;

	  private:
		/**
		 * Made private to hide it from user (ModuleOut should always belong to a Module).
		 */
		void
		operator<< (NoDataSource)
		{ }

	  private:
		Module* _module;
	};


template<class V>
	inline
	ModuleOut<V>::ModuleOut (Module* owner_and_data_source, std::string_view const& path):
		BasicModuleOut (owner_and_data_source, path)
	{
		if (!owner_and_data_source)
			throw xf::InvalidArgument ("ModuleOut requires non-null module pointer");

		_module = owner_and_data_source;
		Module::ModuleSocketAPI (*_module).register_output_socket (*this);
	}


template<class V>
	inline
	ModuleOut<V>::~ModuleOut()
	{
		// TODO previously all targets were set to << no_data_source; before deregister() - is this safe to do this in ~ConnectableSocket()?

		deregister();
	}


template<class V>
	inline void
	ModuleOut<V>::do_fetch (Cycle const& cycle)
	{
		if (_module)
			Module::ProcessingLoopAPI (*_module).fetch_and_process (cycle);
		else
			this->protected_set_nil();
	}


template<class V>
	inline void
	ModuleOut<V>::deregister()
	{
		if (_module)
			Module::ModuleSocketAPI (*_module).unregister_output_socket (*this);

		// Order is important:
		(*this) << no_data_source;
		this->_module = nullptr;
	}

} // namespace xf

#endif

