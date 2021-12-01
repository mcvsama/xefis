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

#ifndef XEFIS__CORE__SOCKETS__MODULE_IN_H__INCLUDED
#define XEFIS__CORE__SOCKETS__MODULE_IN_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdint>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/basic_module_in.h>
#include <xefis/core/sockets/connectable_socket.h>


namespace xf {

/**
 * ModuleSocket that acts as an input to the module.
 * Mostly like Socket.
 */
template<class pValue>
	class ModuleIn final:
		public BasicModuleIn,
		public ConnectableSocket<pValue>
	{
	  public:
		typedef pValue Value;

	  public:
		/**
		 * Create a ModuleIn that's coupled to given owner, but doesn't have any data source yet.
		 */
		explicit
		ModuleIn (ModuleIO* owner, std::string_view const& path);

		/**
		 * Same as ModuleIn (ModuleIO*, std::string_view), but additionally set up the fallback value.
		 */
		explicit
		ModuleIn (ModuleIO* owner, std::string_view const& path, Value fallback_value);

		// Dtor
		~ModuleIn()
			{ deregister(); }

		// Forbid copying
		ModuleIn<Value>&
		operator= (ModuleIn<Value> const&) = delete;

		// Move operator
		[[nodiscard]]
		ModuleIn<Value>&
		operator= (ModuleIn<Value>&&) = default;

		// BasicModuleSocket API
		void
		deregister() override;
	};


template<class V>
	inline
	ModuleIn<V>::ModuleIn (ModuleIO* owner, std::string_view const& path):
		BasicModuleIn (owner, path)
	{
		ModuleIO::ProcessingLoopAPI (*this->io()).register_input_socket (*this);
	}


template<class V>
	inline
	ModuleIn<V>::ModuleIn (ModuleIO* owner, std::string_view const& path, Value fallback_value):
		ModuleIn (owner, path)
	{
		this->set_fallback (fallback_value);
	}


template<class V>
	inline void
	ModuleIn<V>::deregister()
	{
		if (this->io())
			ModuleIO::ProcessingLoopAPI (*this->io()).unregister_input_socket (*this);

		// Order is important:
		(*this) << no_data_source;
		this->_owner = nullptr;
	}

} // namespace xf

#endif

