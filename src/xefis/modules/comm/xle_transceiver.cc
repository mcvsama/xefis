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

// Local:
#include "xle_transceiver.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/exception_support.h>
#include <neutrino/string.h>

// Boost:
#include <boost/random.hpp>
#include <boost/random/random_device.hpp>

// Standard:
#include <cstddef>
#include <future>


namespace xf::crypto::xle {

static Blob const kMasterToSlave { 0x01 };
static Blob const kSlaveToMaster { 0x02 };

static thread_local boost::random::random_device transceiver_rnd;


Transceiver::Transceiver (Role const role, size_t ciphertext_expansion, xf::Logger const& logger):
	_role (role),
	_logger (logger),
	_ciphertext_expansion (ciphertext_expansion)
{ }


bool
Transceiver::ready() const
{
	if (auto const* active_session = this->active_session();
		active_session && active_session->connected())
	{
		return true;
	}

	if (auto const* next_session_candidate = this->next_session_candidate();
		next_session_candidate && next_session_candidate->connected())
	{
		return true;
	}

	if (auto const* previous_session = this->previous_session();
		previous_session && previous_session->connected())
	{
		return true;
	}

	return false;
}


Blob
Transceiver::encrypt_packet (BlobView const packet)
{
	Session* first_session_to_try = active_session();
	Session* second_session_to_try = next_session_candidate();
	std::string_view first_session_role = "active session";
	std::string_view second_session_role = "next session candidate";

	// Slave should always try the active session first, but Master should first try
	// the next session candidate if it exists, because eventually someone has to
	// try encryption with newly prepared session:
	if (_role == Role::Master && next_session_candidate())
	{
		std::swap (first_session_to_try, second_session_to_try);
		std::swap (first_session_role, second_session_role);
	}

	auto const try_session = [&] (Session* session, std::string_view const session_name, bool shift_sessions, std::function<Blob()> const fallback = {}) -> Blob {
		if (session)
		{
			try {
				auto const blob = session->encrypt_packet (packet);

				if (shift_sessions)
					this->shift_sessions();

				return blob;
			}
			catch (...)
			{
				if (fallback)
					return fallback();
				else
					throw FastException (std::format ("{}: {} thrown an exception; fallback unavailable", role_name(), session_name), std::current_exception());
			}
		}
		else if (fallback)
			return fallback();
		else
			throw FastException (std::format ("{}: {} is unavailable; fallback is unavailable", role_name(), session_name));
	};

	return try_session (first_session_to_try, first_session_role, _role == Role::Master, [&] {
		return try_session (second_session_to_try, second_session_role, false);
	});
}


Blob
Transceiver::decrypt_packet (BlobView const packet, std::optional<Transport::SequenceNumber> const maximum_allowed_sequence_number)
{
	auto const try_session = [&] (Session* session, std::string_view const session_name, bool shift_sessions, bool get_rid_of_previous_session, std::function<Blob()> const fallback = {}) -> Blob {
		if (session)
		{
			try {
				auto const blob = session->decrypt_packet (packet, maximum_allowed_sequence_number);

				if (shift_sessions)
					this->shift_sessions();

				if (get_rid_of_previous_session)
					this->get_rid_of_previous_session();

				return blob;
			}
			catch (...)
			{
				if (fallback)
					return fallback();
				else
					throw FastException (std::format ("{}: {} thrown an exception, fallback unavailable", role_name(), session_name), std::current_exception());
			}
		}
		else if (fallback)
			return fallback();
		else
			throw FastException (std::format ("{}: {} is unavailable; fallback is unavailable", role_name(), session_name));
	};

	return try_session (active_session(), "active session", false, true, [&] {
		return try_session (previous_session(), "previous session", false, false, [&] {
			return try_session (next_session_candidate(), "next session candidate", true, false);
		});
	});
}


std::string
Transceiver::role_name() const
{
	switch (_role)
	{
		case Role::Master:	return "MasterTransceiver";
		case Role::Slave:	return "SlaveTransceiver";
	}

	return {};
}


MasterTransceiver::Session::HandshakeRequested::HandshakeRequested (CryptoParams const& params):
	handshake_master (transceiver_rnd, {
		.master_signature_key = params.master_signature_key,
		.slave_signature_key = params.slave_signature_key,
		.hmac_size = params.hmac_size,
		.max_time_difference = params.max_time_difference,
	}),
	handshake_request (handshake_master.generate_handshake_blob (neutrino::TimeHelper::now()))
{ }


MasterTransceiver::Session::Connected::Connected (Secure<Blob> const& ephemeral_key, CryptoParams const& params):
	transmitter (transceiver_rnd, {
		.ephemeral_session_key = *ephemeral_key,
		.authentication_secret = params.authentication_secret,
		.data_encryption_secret = params.data_encryption_secret,
		.seq_num_encryption_secret = params.seq_num_encryption_secret,
		.hmac_size = params.hmac_size,
		.hkdf_user_info = kMasterToSlave,
	}),
	receiver ({
		.ephemeral_session_key = *ephemeral_key,
		.authentication_secret = params.authentication_secret,
		.data_encryption_secret = params.data_encryption_secret,
		.seq_num_encryption_secret = params.seq_num_encryption_secret,
		.hmac_size = params.hmac_size,
		.hkdf_user_info = kSlaveToMaster,
	})
{ }


MasterTransceiver::Session::Session (CryptoParams const& params):
	Transceiver::Session ("M", _id_generator),
	_crypto_params (params),
	_state (std::in_place_type_t<HandshakeRequested>(), params)
{
	_session_prepared_future = _session_prepared_promise.get_future();
	_session_activated_future = _session_activated_promise.get_future();
}


MasterTransceiver::Session::~Session()
{
	abort (AbortReason::Deleted);
}


Blob const&
MasterTransceiver::Session::handshake_request() const
{
	if (auto* hs = std::get_if<HandshakeRequested> (&_state))
		return hs->handshake_request;
	else
		throw std::runtime_error ("handshake request unavailable; current state is not HandshakeResponded");
}


void
MasterTransceiver::Session::set_handshake_response (Blob const& handshake_response)
{
	if (auto* hr = std::get_if<HandshakeRequested> (&_state))
	{
		auto const ephemeral_key = Secure (hr->handshake_master.calculate_key (handshake_response));
		_state.emplace<Connected> (ephemeral_key, _crypto_params);
		_session_prepared_promise.set_value();
	}
	else
		throw FastException ("unexpected MasterTransceiver::Session::set_handshake_response() when not waiting for it");
}


void
MasterTransceiver::Session::abort (AbortReason const reason)
{
	if (!is_ready (_session_prepared_future))
		_session_prepared_promise.set_exception (std::make_exception_ptr (HandshakeAborted({ reason })));

	if (!is_ready (_session_activated_future))
		_session_activated_promise.set_exception (std::make_exception_ptr (HandshakeAborted({ reason })));
}


std::optional<Blob>
MasterTransceiver::Session::tx_key_hash() const
{
	if (auto *connected = std::get_if<Connected> (&_state))
		return connected->transmitter.data_encryption_key_hash();
	else
		return std::nullopt;
}


std::optional<Blob>
MasterTransceiver::Session::rx_key_hash() const
{
	if (auto *connected = std::get_if<Connected> (&_state))
		return connected->receiver.data_encryption_key_hash();
	else
		return std::nullopt;
}


Transmitter&
MasterTransceiver::Session::transmitter()
{
	if (auto* connected = std::get_if<Connected> (&_state))
		return connected->transmitter;
	else
		throw FastException ("failed to encrypt packet: master transceiver not connected (handshake not finalized)");
}


Receiver&
MasterTransceiver::Session::receiver()
{
	if (auto* connected = std::get_if<Connected> (&_state))
		return connected->receiver;
	else
		throw FastException ("failed to decrypt packet: master transceiver not connected (handshake not finalized)");
}


MasterTransceiver::MasterTransceiver (CryptoParams const& params, xf::Logger const& logger, std::string_view const& instance):
	Transceiver (Role::Master, Transport::ciphertext_expansion (params.hmac_size), logger.with_scope (std::string (kLoggerScope) + "#" + instance)),
	Module ("MasterTransceiver"),
	_crypto_params (params)
{ }


MasterTransceiver::StartHandshakeResult
MasterTransceiver::start_handshake()
{
	_next_session_candidate = std::make_unique<Session> (_crypto_params);
	this->handshake_request = to_string (_next_session_candidate->handshake_request());

	return {
		.session_prepared	= _next_session_candidate->session_prepared(),
		.session_activated	= _next_session_candidate->session_activated(),
	};
}


std::optional<Blob>
MasterTransceiver::tx_key_hash() const
{
	if (_active_session)
		return _active_session->tx_key_hash();
	else
		return std::nullopt;
}


std::optional<Blob>
MasterTransceiver::rx_key_hash() const
{
	if (_active_session)
		return _active_session->rx_key_hash();
	else
		return std::nullopt;
}


void
MasterTransceiver::process (Cycle const&)
{
	if (_start_handshake_button_changed.value_changed() && this->start_handshake_button.value_or (false))
		start_handshake();

	try {
		if (_handshake_response_changed.value_changed() && this->handshake_response.valid())
			if (_next_session_candidate->waiting_for_handshake_response())
				_next_session_candidate->set_handshake_response (to_blob (*this->handshake_response));
	}
	catch (...)
	{
		logger() << std::format ("Exception when handling handshake response: {}\n", neutrino::describe_exception (std::current_exception()));
	}
}


void
MasterTransceiver::shift_sessions()
{
	if (_next_session_candidate)
	{
		_previous_session = std::move (_active_session);
		_active_session = std::move (_next_session_candidate);
		this->handshake_request = xf::nil;
		_active_session->set_activated();
	}
}


SlaveTransceiver::Session::Session (Blob const& handshake_request,
									CryptoParams const& params,
									std::function<bool (HandshakeID)> handshake_id_reused_check):
	Transceiver::Session ("S", _id_generator)
{
	auto handshake_slave = HandshakeSlave (transceiver_rnd, {
		.master_signature_key = params.master_signature_key,
		.slave_signature_key = params.slave_signature_key,
		.hmac_size = params.hmac_size,
		.max_time_difference = params.max_time_difference,
	}, handshake_id_reused_check);
	auto const response_and_key = handshake_slave.generate_handshake_blob_and_key (handshake_request, neutrino::TimeHelper::now());
	_handshake_response = response_and_key.handshake_response;
	_transmitter.emplace (transceiver_rnd, Transmitter::Params {
		.ephemeral_session_key = *response_and_key.ephemeral_key,
		.authentication_secret = params.authentication_secret,
		.data_encryption_secret = params.data_encryption_secret,
		.seq_num_encryption_secret = params.seq_num_encryption_secret,
		.hmac_size = params.hmac_size,
		.hkdf_user_info = kSlaveToMaster,
	});
	_receiver.emplace (Receiver::Params {
		.ephemeral_session_key = *response_and_key.ephemeral_key,
		.authentication_secret = params.authentication_secret,
		.data_encryption_secret = params.data_encryption_secret,
		.seq_num_encryption_secret = params.seq_num_encryption_secret,
		.hmac_size = params.hmac_size,
		.hkdf_user_info = kMasterToSlave,
	});
}


SlaveTransceiver::SlaveTransceiver (CryptoParams const& params,
									std::function<bool (HandshakeID)> handshake_id_reuse_check,
									xf::Logger const& logger,
									std::string_view const& instance):
	Transceiver (Role::Slave, Transport::ciphertext_expansion (params.hmac_size), logger.with_scope (std::string (kLoggerScope) + "#" + instance)),
	Module ("SlaveTransceiver"),
	_crypto_params (params),
	_handshake_id_reuse_check (handshake_id_reuse_check)
{ }


std::optional<Blob>
SlaveTransceiver::tx_key_hash() const
{
	if (_active_session)
		return _active_session->tx_key_hash();
	else
		return std::nullopt;
}


std::optional<Blob>
SlaveTransceiver::rx_key_hash() const
{
	if (_active_session)
		return _active_session->rx_key_hash();
	else
		return std::nullopt;
}


void
SlaveTransceiver::process (Cycle const&)
{
	try {
		if (_handshake_request_changed.value_changed() && this->handshake_request.valid())
		{
			_next_session_candidate = std::make_unique<Session> (to_blob (*this->handshake_request), _crypto_params, _handshake_id_reuse_check);
			this->handshake_response = to_string (*_next_session_candidate->handshake_response());
		}
	}
	catch (...)
	{
		logger() << std::format ("Exception when handling handshake request: {}\n", neutrino::describe_exception (std::current_exception()));
	}
}


void
SlaveTransceiver::shift_sessions()
{
	if (_next_session_candidate)
	{
		_active_session = std::move (_next_session_candidate);
		this->handshake_response = xf::nil;
	}
}

} // namespace xf::crypto::xle

