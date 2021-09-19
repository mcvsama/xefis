/* vim:ts=4
 *
 * Copyleft 2020  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__CONNECTABLE_SOCKET_H__INCLUDED
#define XEFIS__CORE__CONNECTABLE_SOCKET_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/socket.h>


namespace xf {

/**
 * A Socket that can use other sockets and constant values as data source.
 */
template<class pValue>
	class ConnectableSocket: public Socket<pValue>
	{
	  public:
		using Value = pValue;

	  private:
		using SourceVariant = std::variant<
			// Not connected to any source (giving nil values):
			std::monostate,
			// Constant value source:
			ConstantSource<Value>,
			// Non-owned socket (eg. ModuleOuts of Modules):
			Socket<Value>*,
			// Owned socket (filters in chains, etc.):
			std::unique_ptr<Socket<Value>>
		>;

	  public:
		using Socket<Value>::Socket;

		// Dtor
		~ConnectableSocket();

		// BasicSocket API
		void
		operator<< (NoDataSource) override;

		/**
		 * Set non-owned Socket as a data source for this socket.
		 */
		template<template<class> class SocketType>
			requires (std::is_base_of_v<Socket<Value>, SocketType<Value>>)
			void
			operator<< (SocketType<Value>&);

		/**
		 * Set owned Socket as a data source for this socket.
		 */
		template<template<class> class SocketType>
			requires (std::is_base_of_v<Socket<Value>, SocketType<Value>>)
			SocketType<Value>&
			operator<< (std::unique_ptr<SocketType<Value>>&&);

		/**
		 * Set a constant value as a data source for this socket.
		 */
		template<class CompatibleValue>
			requires (std::is_convertible_v<CompatibleValue, Value> || si::is_quantity_v<CompatibleValue>)
			void
			operator<< (ConstantSource<CompatibleValue> const&);

		/**
		 * Set a constant value as a data source for this socket.
		 */
		template<class CompatibleValue>
			requires (std::is_convertible_v<CompatibleValue, Value> || si::is_quantity_v<CompatibleValue>)
			void
			operator<< (CompatibleValue const& compatible_value)
				{ return *this << ConstantSource<Value> (static_cast<Value> (compatible_value)); }

	  protected:
		// BasicSocket API
		void
		do_fetch (Cycle const&) override;

	  private:
		void
		inc_source_use_count();

		void
		dec_source_use_count();

		/**
		 * Fetch data from given socket.
		 */
		void
		fetch_from_socket (Socket<Value>&, Cycle const&);

	  private:
		SourceVariant _source;
	};


/**
 * Returns logger to use for exceptions thrown when fetching data
 * from connected sockets.
 * By default it's Xefis::fallback_exception_logger().
 */
Logger const*
connectable_socket_fetch_exception_logger();

/**
 * Set new logger to return by connectable_socket_fetch_exception_logger().
 * May be nullptr to hide exception.
 */
void
set_connectable_socket_fetch_exception_logger (Logger const*);


template<class V>
	ConnectableSocket<V>::~ConnectableSocket()
	{
		// Unregister ourselves from the source:
		dec_source_use_count();
	}


template<class V>
	inline void
	ConnectableSocket<V>::operator<< (NoDataSource const)
	{
		dec_source_use_count();
		_source = std::monostate{};
		this->protected_set_nil();
	}


template<class V>
	template<template<class> class SocketType>
		requires (std::is_base_of_v<Socket<V>, SocketType<V>>)
		inline void
		ConnectableSocket<V>::operator<< (SocketType<Value>& source)
		{
			dec_source_use_count();
			_source = &source;
			inc_source_use_count();
			this->protected_set (source);
		}


template<class V>
	template<template<class> class SocketType>
		requires (std::is_base_of_v<Socket<V>, SocketType<V>>)
		inline SocketType<V>&
		ConnectableSocket<V>::operator<< (std::unique_ptr<SocketType<Value>>&& source)
		{
			dec_source_use_count();
			_source = std::move (source);
			inc_source_use_count();
			auto& uptr_ref = std::get<std::unique_ptr<Socket<Value>>> (_source);
			this->protected_set (*uptr_ref);
			return static_cast<SocketType<V>&> (*uptr_ref);
		}


template<class V>
	template<class CompatibleValue>
		requires (std::is_convertible_v<CompatibleValue, V> || si::is_quantity_v<CompatibleValue>)
		inline void
		ConnectableSocket<V>::operator<< (ConstantSource<CompatibleValue> const& source)
		{
			dec_source_use_count();
			_source = ConstantSource<Value> { source.value };
			inc_source_use_count();
			this->protected_set (source.value);
		}


template<class V>
	inline void
	ConnectableSocket<V>::do_fetch (Cycle const& cycle)
	{
		std::visit (overload {
			[&] (std::monostate) {
				this->protected_set_nil();
			},
			[&] (ConstantSource<Value>& constant_source) {
				this->protected_set (constant_source.value);
			},
			[&] (Socket<Value>* socket) {
				fetch_from_socket (*socket, cycle);
			},
			[&] (std::unique_ptr<Socket<Value>>& socket) {
				fetch_from_socket (*socket, cycle);
			}
		}, _source);
	}


template<class V>
	inline void
	ConnectableSocket<V>::inc_source_use_count()
	{
		std::visit (overload {
			[&] (std::monostate) noexcept {
				// No action
			},
			[&] (ConstantSource<Value>&) noexcept {
				// No action
			},
			[&] (Socket<Value>* socket) {
				socket->inc_use_count (this);
			},
			[&] (std::unique_ptr<Socket<Value>>& socket) {
				socket->inc_use_count (this);
			}
		}, _source);
	}


template<class V>
	inline void
	ConnectableSocket<V>::dec_source_use_count()
	{
		std::visit (overload {
			[&] (std::monostate) noexcept {
				// No action
			},
			[&] (ConstantSource<Value>&) noexcept {
				// No action
			},
			[&] (Socket<Value>* socket) {
				socket->dec_use_count (this);
			},
			[&] (std::unique_ptr<Socket<Value>>& socket) {
				socket->dec_use_count (this);
			}
		}, _source);
	}


template<class V>
	inline void
	ConnectableSocket<V>::fetch_from_socket (Socket<Value>& socket, Cycle const& cycle)
	{
		auto const execute = [&] {
			socket.fetch (cycle);

			if (auto const new_value = socket.get_optional())
				this->protected_set (*new_value);
			else
			{
				// Propagate nil-by-fetch-exception flag from the source socket:
				this->set_nil_by_fetch_exception (socket.nil_by_fetch_exception());
				this->protected_set_nil();
			}
		};

		bool thrown = false;
		this->set_nil_by_fetch_exception (false);

		if (auto const* logger = connectable_socket_fetch_exception_logger())
			thrown = Exception::catch_and_log (*logger, execute);
		else
		{
			try {
				execute();
			}
			catch (...)
			{
				thrown = true;
			}
		}

		if (thrown)
			this->set_nil_by_fetch_exception (true);
	}

} // namespace xf

#endif

