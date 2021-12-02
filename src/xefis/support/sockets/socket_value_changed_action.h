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

#ifndef XEFIS__SUPPORT__SOCKETS__SOCKET_VALUE_CHANGED_ACTION_H__INCLUDED
#define XEFIS__SUPPORT__SOCKETS__SOCKET_VALUE_CHANGED_ACTION_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>
#include <optional>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/sockets/socket_action.h>
#include <xefis/support/sockets/socket_value_changed.h>


namespace xf {

/**
 * Fires a callback when socket's value change.
 */
template<class pValue>
	class SocketValueChangedAction:
		public SocketValueChanged<pValue>,
		public SocketAction
	{
	  public:
		using Value		= pValue;
		using Action	= std::function<void (std::optional<Value> const&)>;

	  public:
		// Ctor
		explicit
		SocketValueChangedAction (Socket<Value>&, Action);

		// SocketAction API
		void
		process() override;

	  private:
		Action _action;
	};


template<class V>
	inline
	SocketValueChangedAction<V>::SocketValueChangedAction (Socket<Value>& socket, Action const action):
		SocketValueChanged<V> (socket),
		_action (action)
	{ }


template<class V>
	inline void
	SocketValueChangedAction<V>::process()
	{
		if (this->value_changed())
			_action (this->socket().get_optional());
	}

} // namespace xf

#endif

