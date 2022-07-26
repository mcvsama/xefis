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

#ifndef XEFIS__CORE__SOCKETS__BASIC_SOCKET_H__INCLUDED
#define XEFIS__CORE__SOCKETS__BASIC_SOCKET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/cycle.h>
#include <xefis/core/sockets/common.h>
#include <xefis/core/sockets/socket_converter.h>

// Neutrino:
#include <neutrino/noncopyable.h>
#include <neutrino/nonmovable.h>
#include <neutrino/numeric.h>
#include <neutrino/variant.h>
#include <neutrino/utility.h>

// Standard:
#include <variant>


namespace xf {

/**
 * A value holder.
 *
 * TODO Note: perhaps the *_age() methods should not use the timestamp of the set() call, but some timestamp provided from
 * outside, eg. some souce data sampling timestamp. That would be more proper from digital signal processing
 * perspective, but I guess it's OK enough as it is now.
 */
class BasicSocket:
	private Noncopyable,
	private Nonmovable
{
	template<class Value>
		friend class Socket;

	template<class Value, class AssignedValue>
		friend class ConnectableSocket;

  public:
	// Used to tell if node value has changed:
	typedef uint64_t Serial;

  public:
	// Ctor
	BasicSocket();

	// Dtor
	virtual
	~BasicSocket();

	// Move operator

	/**
	 * Ensure that socket's value is up to date in this processing loop.
	 */
	void
	fetch (Cycle const&);

	/**
	 * Set no data source for this socket.
	 */
	virtual void
	operator<< (NoDataSource) = 0;

	/**
	 * Return true if socket is nil.
	 * If a fallback-value is set, it will never return true.
	 */
	[[nodiscard]]
	virtual bool
	is_nil() const noexcept = 0;

	/**
	 * Alias for is_nil().
	 */
	[[nodiscard]]
	bool
	operator== (Nil) const
		{ return is_nil(); }

	/**
	 * Valid means not nil. Equivalent to !is_nil().
	 */
	[[nodiscard]]
	virtual bool
	valid() const noexcept = 0;

	/**
	 * Alias for valid().
	 */
	[[nodiscard]]
	operator bool() const noexcept
		{ return valid(); }

	/**
	 * Return the serial value of the socket.
	 * Serial value changes when socket is updated.
	 */
	[[nodiscard]]
	Serial
	serial() const noexcept
		{ return _serial; }

	/**
	 * Return timestamp of the value (time when it was modified).
	 */
	[[nodiscard]]
	si::Time
	modification_timestamp() const noexcept
		{ return _modification_timestamp; }

	/**
	 * Return age of the value (time since it was last modified).
	 */
	[[nodiscard]]
	si::Time
	modification_age() const noexcept
		{ return TimeHelper::now() - modification_timestamp(); }

	/**
	 * Return timestamp of the last non-nil value.
	 */
	[[nodiscard]]
	si::Time
	valid_timestamp() const noexcept
		{ return _valid_timestamp; }

	/**
	 * Return age of the non-nil value (time since it was last set to a non-nil value).
	 * Setting a fallback-value will essentially mean setting not-nil.
	 */
	[[nodiscard]]
	si::Time
	valid_age() const noexcept
		{ return TimeHelper::now() - valid_timestamp(); }

	/**
	 * Use-count for this socket.
	 * This is number of sockets reading value from this socket.
	 */
	[[nodiscard]]
	std::size_t
	use_count() const noexcept
		{ return _targets.size(); }

	/**
	 * Return true if Blob returned by to_blob() is constant size.
	 */
	[[nodiscard]]
	virtual bool
	has_constant_blob_size() const noexcept = 0;

	/**
	 * Return Blob size for this Socket, provided that has_constant_blob_size() is true.
	 * If has_constant_blob_size() gives false, return 0.
	 * TODO or perhaps throw std::logic_error?
	 */
	[[nodiscard]]
	virtual size_t
	constant_blob_size() const noexcept = 0;

	/**
	 * Serialize socket's value to string with given config.
	 */
	[[nodiscard]]
	virtual std::string
	to_string (SocketConversionSettings const& = {}) const = 0;

	/**
	 * Extract numeric value from the socket, if applies.
	 */
	[[nodiscard]]
	virtual std::optional<float128_t>
	to_floating_point (SocketConversionSettings const& = {}) const = 0;

	/**
	 * Serializes socket's value, including nil-flag.
	 */
	[[nodiscard]]
	virtual Blob
	to_blob() const = 0;

	/**
	 * True if currently held nil value was caused by exception
	 * thrown by source socket when fetching data from it.
	 * Will get reset on next successful fetch.
	 */
	[[nodiscard]]
	bool
	nil_by_fetch_exception() const
		{ return _nil_by_fetch_exception; }

  protected:
	/**
	 * Fetch the data from the source unconditionally.
	 */
	virtual void
	do_fetch (Cycle const&) = 0;

	/**
	 * Increase use-count of this socket (listener started listening to value of this socket).
	 */
	void
	inc_use_count (BasicSocket* listener)
		{ _targets.push_back (listener); }

	/**
	 * Decrease use-count of this socket (listener stopped listening to value of this socket).
	 */
	void
	dec_use_count (BasicSocket* listener);

	/**
	 * Set socket to the nil value.
	 */
	virtual void
	protected_set_nil() = 0;

	/**
	 * Set the nil-by-fetch-exception flag.
	 */
	void
	set_nil_by_fetch_exception (bool value)
		{ _nil_by_fetch_exception = value; }

  private:
	si::Time					_modification_timestamp	= 0_s;
	si::Time					_valid_timestamp		= 0_s;
	Serial						_serial					= 0;
	Cycle::Number				_fetched_cycle_number	= 0;
	std::vector<BasicSocket*>	_targets;
	bool						_nil_by_fetch_exception = false;
};


inline
BasicSocket::BasicSocket()
{
	_targets.reserve (8);
}


inline
BasicSocket::~BasicSocket()
{
	for (auto* target: clone (_targets))
		(*target) << no_data_source;
}


inline void
BasicSocket::fetch (Cycle const& cycle)
{
	if (_fetched_cycle_number < cycle.number())
	{
		_fetched_cycle_number = cycle.number();
		do_fetch (cycle);
	}
}


inline void
BasicSocket::dec_use_count (BasicSocket* listener)
{
	auto new_end = std::remove (_targets.begin(), _targets.end(), listener);
	_targets.resize (neutrino::to_unsigned (std::distance (_targets.begin(), new_end)));
}

} // namespace xf

#endif

