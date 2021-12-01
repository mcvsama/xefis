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

// Standard:
#include <cstddef>
#include <cstdint>

// Neutrino:
#include <neutrino/blob.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/assignable_socket.h>
#include <xefis/core/sockets/basic_module_out.h>


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
		 * Create ModuleOut that's coupled to a ModuleIO and set the module as data source.
		 */
		explicit
		ModuleOut (ModuleIO* owner_and_data_source, std::string_view const& path);

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
		 * Made private to hide it from user (ModuleOut should always belong to a ModuleIO).
		 */
		void
		operator<< (NoDataSource)
		{ }

	  private:
		ModuleIO* _module_io;
	};


template<class V>
	inline
	ModuleOut<V>::ModuleOut (ModuleIO* owner_and_data_source, std::string_view const& path):
		BasicModuleOut (owner_and_data_source, path)
	{
		_module_io = owner_and_data_source;
		ModuleIO::ProcessingLoopAPI (*this->io()).register_output_socket (*this);
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
		if (_module_io)
			BasicModule::ProcessingLoopAPI (_module_io->module()).fetch_and_process (cycle);
		else
			this->protected_set_nil();
	}


template<class V>
	inline void
	ModuleOut<V>::deregister()
	{
		if (this->io())
			ModuleIO::ProcessingLoopAPI (*this->io()).unregister_output_socket (*this);

		// Order is important:
		(*this) << no_data_source;
		this->_owner = nullptr;
	}

} // namespace xf

#endif

