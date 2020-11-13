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

#ifndef XEFIS__UTILITY__ACTIONS_H__INCLUDED
#define XEFIS__UTILITY__ACTIONS_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

/**
 * Checks if an action should be executed based on value of some sockets and saved state.
 */
class PropAction
{
  public:
	/**
	 * Tell whether condition was met to execute an action.
	 */
	virtual bool
	operator()() = 0;
};


class SerialChanged: public PropAction
{
  public:
	using Serial	= BasicSocket::Serial;

  public:
	// Ctor
	explicit
	SerialChanged (BasicSocket& socket):
		_socket (socket)
	{ }

	// PropAction API
	bool
	operator()() override
	{
		auto new_serial = _socket.serial();

		if (new_serial != _serial)
		{
			_serial = new_serial;
			return true;
		}

		return false;
	}

  private:
	Serial			_serial	{ 0 };
	BasicSocket&	_socket;
};


/**
 * Checks whether a socket changed its value since last check.
 */
template<class pValue>
	class PropChanged: public PropAction
	{
	  public:
		using Value			= pValue;
		using OptionalValue	= std::optional<Value>;
		using Socket		= xf::Socket<Value>;

	  public:
		// Ctor
		explicit
		PropChanged (Socket const& socket):
			_socket (socket),
			_last_value (socket.get_optional())
		{ }

		// PropAction API
		bool
		operator()() override
		{
			auto current_value = _socket.get_optional();

			if (_last_value != current_value)
			{
				_last_value = current_value;
				return true;
			}

			return false;
		}

		/**
		 * Return reference to observed socket.
		 */
		Socket const&
		socket() const noexcept
		{
			return _socket;
		}

	  private:
		Socket const&	_socket;
		OptionalValue	_last_value;
	};


/**
 * Checks whether a socket has changed and has now given value.
 */
template<class pValue>
	class PropChangedTo: public PropChanged<pValue>
	{
	  public:
		using Value			= pValue;
		using OptionalValue	= typename PropChanged<Value>::OptionalValue;
		using Socket		= typename PropChanged<Value>::Socket;

	  public:
		// Ctor
		constexpr
		PropChangedTo (Socket const& socket, Value value):
			PropChanged<Value> (socket),
			_expected_value (value)
		{ }

		// PropAction API
		bool
		operator()() override
		{
			return PropChanged<Value>::operator()()
				&& this->socket().valid()
				&& *this->socket() == expected_value();
		}

		/**
		 * Return the value this observer expects.
		 */
		constexpr Value
		expected_value() const noexcept
		{
			return _expected_value;
		}

	  private:
		Value _expected_value;
	};


/**
 * Checks whether a socket changed to nil.
 */
template<class pValue>
	class ChangedToNil: public PropChanged<pValue>
	{
	  public:
		using Value			= pValue;
		using OptionalValue	= typename PropChanged<Value>::OptionalValue;
		using Socket		= typename PropChanged<Value>::Socket;

	  public:
		using PropChanged<Value>::PropChanged;

		// PropAction API
		bool
		operator()() override
		{
			return PropChanged<Value>::operator()
				&& this->socket().is_nil();
		}
	};

} // namespace xf

#endif

