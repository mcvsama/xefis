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

#ifndef XEFIS__CORE__SOCKETS__CONNECTABLE_SOCKET_H__INCLUDED
#define XEFIS__CORE__SOCKETS__CONNECTABLE_SOCKET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/constant_source.h>
#include <xefis/core/sockets/socket.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * A Socket that can use other sockets and constant values as data source.
 */
template<class pObservedValue, class pAssignedValue = pObservedValue>
	class ConnectableSocket: public Socket<pObservedValue>
	{
	  public:
		using ObservedValue = pObservedValue;
		using AssignedValue = pAssignedValue;
		// Functions that transform assigned value before it's actually assigned:
		using Transformer1 = std::function<ObservedValue (AssignedValue)>;
		using Transformer2 = std::function<ObservedValue (std::optional<AssignedValue>)>;
		using Transformer3 = std::function<std::optional<ObservedValue> (AssignedValue)>;
		using Transformer4 = std::function<std::optional<ObservedValue> (std::optional<AssignedValue>)>;
		using Transformer = std::variant<Transformer1, Transformer2, Transformer3, Transformer4>;

	  private:
		using SourceVariant = std::variant<
			// Not connected to any source (giving nil values):
			std::monostate,
			// Constant value source:
			ConstantSource<AssignedValue>,
			// Non-owned socket (eg. ModuleOuts of Modules):
			Socket<AssignedValue>*,
			// Owned socket (filters in chains, etc.):
			std::unique_ptr<Socket<AssignedValue>>
		>;

	  public:
		using Socket<ObservedValue>::Socket;

		/**
		 * Passing empty std::function will cause this socket to always report nil-value.
		 */
		ConnectableSocket (Transformer const& transformer):
			_transformer (transformer)
		{ }

		// Dtor
		~ConnectableSocket();

		// BasicSocket API
		void
		operator<< (NoDataSource) override;

		/**
		 * Set non-owned Socket as a data source for this socket.
		 */
		template<template<class> class SocketType>
			requires (std::is_base_of_v<Socket<AssignedValue>, SocketType<AssignedValue>>)
			SocketType<AssignedValue>&
			operator<< (SocketType<AssignedValue>&);

		/**
		 * Set owned Socket as a data source for this socket.
		 */
		template<template<class> class SocketType>
			requires (std::is_base_of_v<Socket<AssignedValue>, SocketType<AssignedValue>>)
			SocketType<AssignedValue>&
			operator<< (std::unique_ptr<SocketType<AssignedValue>>&&);

		/**
		 * Set a constant value as a data source for this socket.
		 */
		template<class CompatibleValue>
			requires (std::is_convertible_v<CompatibleValue, AssignedValue> || si::is_quantity_v<CompatibleValue>)
			void
			operator<< (ConstantSource<CompatibleValue> const&);

		/**
		 * Set a constant value as a data source for this socket.
		 */
		template<class CompatibleValue>
			requires (std::is_convertible_v<CompatibleValue, AssignedValue> || si::is_quantity_v<CompatibleValue>)
			void
			operator<< (CompatibleValue const& compatible_value)
				{ return *this << ConstantSource<AssignedValue> (static_cast<AssignedValue> (compatible_value)); }

		/**
		 * Set a function to be data source for this socket.
		 * Return a reference to a new Socket that wraps the provided function and can be further chained.
		 */
		template<class FunctionArgument>
			ConnectableSocket<AssignedValue, FunctionArgument>&
			operator<< (std::function<AssignedValue (FunctionArgument)> const&);

		/**
		 * Set a function to be data source for this socket.
		 * Return a reference to a new Socket that wraps the provided function and can be further chained.
		 */
		template<class FunctionArgument>
			ConnectableSocket<AssignedValue, FunctionArgument>&
			operator<< (std::function<AssignedValue (std::optional<FunctionArgument>)> const&);

		/**
		 * Set a function to be data source for this socket.
		 * Return a reference to a new Socket that wraps the provided function and can be further chained.
		 */
		template<class FunctionArgument>
			ConnectableSocket<AssignedValue, FunctionArgument>&
			operator<< (std::function<std::optional<AssignedValue> (FunctionArgument)> const&);

		/**
		 * Set a function to be data source for this socket.
		 * Return a reference to a new Socket that wraps the provided function and can be further chained.
		 */
		template<class FunctionArgument>
			ConnectableSocket<AssignedValue, FunctionArgument>&
			operator<< (std::function<std::optional<AssignedValue> (std::optional<FunctionArgument>)> const&);

	  protected:
		// BasicSocket API
		void
		do_fetch (Cycle const&) override;

		/**
		 * Transform argument with the internal transformer function.
		 */
		std::optional<ObservedValue>
		transform (std::optional<AssignedValue> const&) const;

	  private:
		void
		inc_source_use_count();

		void
		dec_source_use_count();

		/**
		 * Fetch data from given socket.
		 */
		void
		fetch_from_socket (Socket<AssignedValue>&, Cycle const&);

	  private:
		SourceVariant				_source;
		std::optional<Transformer>	_transformer;
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


template<class OV, class AV>
	ConnectableSocket<OV, AV>::~ConnectableSocket()
	{
		// Unregister ourselves from the source:
		dec_source_use_count();
	}


template<class OV, class AV>
	inline void
	ConnectableSocket<OV, AV>::operator<< (NoDataSource const)
	{
		dec_source_use_count();
		_source = std::monostate{};
	}


template<class OV, class AV>
	template<template<class> class SocketType>
		requires (std::is_base_of_v<Socket<AV>, SocketType<AV>>)
		inline SocketType<AV>&
		ConnectableSocket<OV, AV>::operator<< (SocketType<AssignedValue>& source)
		{
			dec_source_use_count();
			_source = &source;
			inc_source_use_count();

			return source;
		}


template<class OV, class AV>
	template<template<class> class SocketType>
		requires (std::is_base_of_v<Socket<AV>, SocketType<AV>>)
		inline SocketType<AV>&
		ConnectableSocket<OV, AV>::operator<< (std::unique_ptr<SocketType<AssignedValue>>&& source)
		{
			dec_source_use_count();
			_source = std::move (source);
			inc_source_use_count();

			auto& uptr_ref = std::get<std::unique_ptr<Socket<AssignedValue>>> (_source);
			return static_cast<SocketType<AssignedValue>&> (*uptr_ref);
		}


template<class OV, class AV>
	template<class CompatibleValue>
		requires (std::is_convertible_v<CompatibleValue, AV> || si::is_quantity_v<CompatibleValue>)
		inline void
		ConnectableSocket<OV, AV>::operator<< (ConstantSource<CompatibleValue> const& source)
		{
			dec_source_use_count();
			_source = ConstantSource<AssignedValue> { source.value };
			inc_source_use_count();
		}


template<class OV, class AV>
	template<class FunctionArgument>
		inline ConnectableSocket<AV, FunctionArgument>&
		ConnectableSocket<OV, AV>::operator<< (std::function<AssignedValue (FunctionArgument)> const& transformer)
		{
			auto u = std::make_unique<ConnectableSocket<AssignedValue, FunctionArgument>> (transformer);
			auto b = std::unique_ptr<Socket<AssignedValue>> (static_cast<Socket<AssignedValue>*> (u.release()));
			return static_cast<ConnectableSocket<AssignedValue, FunctionArgument>&> (*this << std::move (b));
		}


template<class OV, class AV>
	template<class FunctionArgument>
		inline ConnectableSocket<AV, FunctionArgument>&
		ConnectableSocket<OV, AV>::operator<< (std::function<AssignedValue (std::optional<FunctionArgument>)> const& transformer)
		{
			auto u = std::make_unique<ConnectableSocket<AssignedValue, FunctionArgument>> (transformer);
			auto b = std::unique_ptr<Socket<AssignedValue>> (static_cast<Socket<AssignedValue>*> (u.release()));
			return static_cast<ConnectableSocket<AssignedValue, FunctionArgument>&> (*this << std::move (b));
		}


template<class OV, class AV>
	template<class FunctionArgument>
		inline ConnectableSocket<AV, FunctionArgument>&
		ConnectableSocket<OV, AV>::operator<< (std::function<std::optional<AssignedValue> (FunctionArgument)> const& transformer)
		{
			auto u = std::make_unique<ConnectableSocket<AssignedValue, FunctionArgument>> (transformer);
			auto b = std::unique_ptr<Socket<AssignedValue>> (static_cast<Socket<AssignedValue>*> (u.release()));
			return static_cast<ConnectableSocket<AssignedValue, FunctionArgument>&> (*this << std::move (b));
		}


template<class OV, class AV>
	template<class FunctionArgument>
		inline ConnectableSocket<AV, FunctionArgument>&
		ConnectableSocket<OV, AV>::operator<< (std::function<std::optional<AssignedValue> (std::optional<FunctionArgument>)> const& transformer)
		{
			auto u = std::make_unique<ConnectableSocket<AssignedValue, FunctionArgument>> (transformer);
			auto b = std::unique_ptr<Socket<AssignedValue>> (static_cast<Socket<AssignedValue>*> (u.release()));
			return static_cast<ConnectableSocket<AssignedValue, FunctionArgument>&> (*this << std::move (b));
		}


template<class OV, class AV>
	inline void
	ConnectableSocket<OV, AV>::do_fetch (Cycle const& cycle)
	{
		bool thrown = false;
		this->set_nil_by_fetch_exception (false);

		auto const execute = [&] {
			std::visit (overload {
				[&] (std::monostate) {
					this->protected_set_nil();
				},
				[&] (ConstantSource<AssignedValue>& constant_source) {
					this->protected_set (transform (constant_source.value));
				},
				[&] (Socket<AssignedValue>* socket) {
					fetch_from_socket (*socket, cycle);
				},
				[&] (std::unique_ptr<Socket<AssignedValue>>& socket) {
					fetch_from_socket (*socket, cycle);
				}
			}, _source);
		};

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


template<class OV, class AV>
	inline std::optional<OV>
	ConnectableSocket<OV, AV>::transform (std::optional<AssignedValue> const& value) const
	{
		if (_transformer)
		{
			return std::visit (overload {
				[&] (Transformer1 const& transformer) -> std::optional<ObservedValue> {
					if (value)
						return transformer (*value);
					else
						return std::nullopt;
				},
				[&] (Transformer2 const& transformer) -> std::optional<ObservedValue> {
					return transformer (value);
				},
				[&] (Transformer3 const& transformer) -> std::optional<ObservedValue> {
					if (value)
						return transformer (*value);
					else
						return std::nullopt;
				},
				[&] (Transformer4 const& transformer) -> std::optional<ObservedValue> {
					return transformer (value);
				},
			}, *_transformer);
		}
		else if constexpr (std::is_same_v<OV, AV>)
			return value;
		else
			return std::nullopt;
	}


template<class OV, class AV>
	inline void
	ConnectableSocket<OV, AV>::inc_source_use_count()
	{
		std::visit (overload {
			[&] (std::monostate) noexcept {
				// No action
			},
			[&] (ConstantSource<AssignedValue>&) noexcept {
				// No action
			},
			[&] (Socket<AssignedValue>* socket) {
				socket->inc_use_count (this);
			},
			[&] (std::unique_ptr<Socket<AssignedValue>>& socket) {
				socket->inc_use_count (this);
			}
		}, _source);
	}


template<class OV, class AV>
	inline void
	ConnectableSocket<OV, AV>::dec_source_use_count()
	{
		std::visit (overload {
			[&] (std::monostate) noexcept {
				// No action
			},
			[&] (ConstantSource<AssignedValue>&) noexcept {
				// No action
			},
			[&] (Socket<AssignedValue>* socket) {
				socket->dec_use_count (this);
			},
			[&] (std::unique_ptr<Socket<AssignedValue>>& socket) {
				socket->dec_use_count (this);
			}
		}, _source);
	}


template<class OV, class AV>
	inline void
	ConnectableSocket<OV, AV>::fetch_from_socket (Socket<AssignedValue>& socket, Cycle const& cycle)
	{
		socket.fetch (cycle);

		auto const source_value = socket.get_optional();
		auto const transformed_value = transform (source_value);

		this->protected_set (transformed_value);

		// If both before and after transformation results are nil, then also
		// propagate the nil-by-exception flag:
		if (!source_value && !transformed_value)
			this->set_nil_by_fetch_exception (socket.nil_by_fetch_exception());
	}

} // namespace xf

#endif

