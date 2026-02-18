/* vim:ts=4
 *
 * Copyleft 2023  Michał Gawron
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
#include "xle_secure_channel.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/exception_support.h>
#include <neutrino/string.h>

// Boost:
#include <boost/random.hpp>

// Standard:
#include <cstddef>
#include <future>
#include <random>


namespace xf::crypto::xle {

namespace global {

// These need to be used as user-info with HKDF for keys used for encryption.
// Different user-info for master-to-slave and slave-to-master channels are
// required to avoid using the same key in both ways (keys should never be
// reused for different purposes).
static Blob const kMasterToSlave { 0x01 };
static Blob const kSlaveToMaster { 0x02 };

static thread_local std::random_device transceiver_rnd ("hw");

} // namespace global


SecureChannel::SecureChannel (Role const role, size_t ciphertext_expansion, nu::Logger const& logger):
	_role (role),
	_logger (logger),
	_ciphertext_expansion (ciphertext_expansion)
{ }


bool
SecureChannel::ready() const
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
SecureChannel::encrypt_packet (BlobView const packet)
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
					std::throw_with_nested (nu::Exception (std::format ("{}: {} thrown an exception; fallback unavailable", role_name(), session_name), false));
			}
		}
		else if (fallback)
			return fallback();
		else
			throw nu::Exception (std::format ("{}: {} is unavailable; fallback is unavailable", role_name(), session_name), false);
	};

	return try_session (first_session_to_try, first_session_role, _role == Role::Master, [&]{
		return try_session (second_session_to_try, second_session_role, false);
	});
}


Blob
SecureChannel::decrypt_packet (BlobView const packet, std::optional<Transport::SequenceNumber> const maximum_allowed_sequence_number)
{
	auto const try_session = [&] (Session* session, std::string_view const session_name, bool shift_sessions, bool get_rid_of_previous_session, std::function<Blob (std::string)> const fallback = {}) -> Blob {
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
					return fallback (std::format ("{}: {}", session_name, nu::describe_exception (std::current_exception())));
				else
					std::throw_with_nested (nu::Exception (std::format ("{}: {} thrown an exception, fallback unavailable", role_name(), session_name), false));
			}
		}
		else if (fallback)
			return fallback (std::format ("{} unavailable", session_name));
		else
			throw nu::Exception (std::format ("{}: {} is unavailable; fallback is unavailable", role_name(), session_name), false);
	};

	return try_session (active_session(), "active session", false, true, [&] (std::string const active_fallback_reason) -> Blob {
		return try_session (previous_session(), "previous session", false, false, [&] (std::string const previous_fallback_reason) -> Blob {
			return try_session (next_session_candidate(), "next session candidate", true, false, [&] (std::string const next_fallback_reason) -> Blob {
				throw nu::Exception (std::format ("{}: {}; {}; {}", role_name(), active_fallback_reason, previous_fallback_reason, next_fallback_reason), false);
			});
		});
	});
}


std::string
SecureChannel::role_name() const
{
	switch (_role)
	{
		case Role::Master:	return "MasterSecureChannel";
		case Role::Slave:	return "SlaveSecureChannel";
	}

	return {};
}


MasterSecureChannel::Session::HandshakeRequested::HandshakeRequested (CryptoParams const& params):
	handshake_master (global::transceiver_rnd, {
		.master_signature_key = params.master_signature_key,
		.slave_signature_key = params.slave_signature_key,
		.hmac_size = params.hmac_size,
		.max_time_difference = params.max_time_difference,
	}),
	handshake_request (handshake_master.generate_handshake_blob (nu::utc_now()))
{ }


MasterSecureChannel::Session::Connected::Connected (nu::Secure<Blob> const& ephemeral_key, CryptoParams const& params):
	transmitter (global::transceiver_rnd, {
		.ephemeral_session_key = *ephemeral_key,
		.authentication_secret = params.authentication_secret,
		.data_encryption_secret = params.data_encryption_secret,
		.seq_num_encryption_secret = params.seq_num_encryption_secret,
		.hmac_size = params.hmac_size,
		// This ensures actual keys are different for both directions:
		.hkdf_user_info = global::kMasterToSlave,
	}),
	receiver ({
		.ephemeral_session_key = *ephemeral_key,
		.authentication_secret = params.authentication_secret,
		.data_encryption_secret = params.data_encryption_secret,
		.seq_num_encryption_secret = params.seq_num_encryption_secret,
		.hmac_size = params.hmac_size,
		// This ensures actual keys are different for both directions:
		.hkdf_user_info = global::kSlaveToMaster,
	})
{ }


MasterSecureChannel::Session::Session (CryptoParams const& params):
	SecureChannel::Session ("M", _id_generator),
	_crypto_params (params),
	_state (std::in_place_type_t<HandshakeRequested>(), params)
{
	_session_prepared_future = _session_prepared_promise.get_future();
	_session_activated_future = _session_activated_promise.get_future();
}


MasterSecureChannel::Session::~Session()
{
	abort (AbortReason::Deleted);
}


Blob const&
MasterSecureChannel::Session::handshake_request() const
{
	if (auto* hs = std::get_if<HandshakeRequested> (&_state))
		return hs->handshake_request;
	else
		throw std::runtime_error ("handshake request unavailable; current state is not HandshakeResponded");
}


void
MasterSecureChannel::Session::set_handshake_response (Blob const& handshake_response)
{
	if (auto* hr = std::get_if<HandshakeRequested> (&_state))
	{
		auto const ephemeral_key = nu::Secure (hr->handshake_master.compute_key (handshake_response));
		_state.emplace<Connected> (ephemeral_key, _crypto_params);
		_session_prepared_promise.set_value();
	}
	else
		throw nu::Exception ("unexpected MasterSecureChannel::Session::set_handshake_response() when not waiting for it", false);
}


void
MasterSecureChannel::Session::abort (AbortReason const reason)
{
	if (!nu::ready (_session_prepared_future))
		_session_prepared_promise.set_exception (std::make_exception_ptr (HandshakeAborted({ reason })));

	if (!nu::ready (_session_activated_future))
		_session_activated_promise.set_exception (std::make_exception_ptr (HandshakeAborted({ reason })));
}


std::optional<Blob>
MasterSecureChannel::Session::tx_key_hash() const
{
	if (auto *connected = std::get_if<Connected> (&_state))
		return connected->transmitter.data_encryption_key_hash();
	else
		return std::nullopt;
}


std::optional<Blob>
MasterSecureChannel::Session::rx_key_hash() const
{
	if (auto *connected = std::get_if<Connected> (&_state))
		return connected->receiver.data_encryption_key_hash();
	else
		return std::nullopt;
}


Transmitter&
MasterSecureChannel::Session::transmitter()
{
	if (auto* connected = std::get_if<Connected> (&_state))
		return connected->transmitter;
	else
		throw nu::Exception ("failed to encrypt packet: master transceiver not connected (handshake not finalized)", false);
}


Receiver&
MasterSecureChannel::Session::receiver()
{
	if (auto* connected = std::get_if<Connected> (&_state))
		return connected->receiver;
	else
		throw nu::Exception ("failed to decrypt packet: master transceiver not connected (handshake not finalized)", false);
}


MasterSecureChannel::MasterSecureChannel (xf::ProcessingLoop& loop, CryptoParams const& params, nu::Logger const& logger, std::string_view const instance):
	SecureChannel (Role::Master, Transport::ciphertext_expansion (params.hmac_size), logger.with_context (std::string (kLoggerScope) + "#" + instance)),
	Module (loop, "MasterSecureChannel"),
	_crypto_params (params)
{ }


MasterSecureChannel::StartHandshakeResult
MasterSecureChannel::start_handshake()
{
	_next_session_candidate = std::make_unique<Session> (_crypto_params);
	this->handshake_request = nu::to_string (_next_session_candidate->handshake_request());

	return {
		.session_prepared	= _next_session_candidate->session_prepared(),
		.session_activated	= _next_session_candidate->session_activated(),
	};
}


std::optional<Blob>
MasterSecureChannel::tx_key_hash() const
{
	if (_active_session)
		return _active_session->tx_key_hash();
	else
		return std::nullopt;
}


std::optional<Blob>
MasterSecureChannel::rx_key_hash() const
{
	if (_active_session)
		return _active_session->rx_key_hash();
	else
		return std::nullopt;
}


void
MasterSecureChannel::process (Cycle const&)
{
	if (_start_handshake_button_changed.value_changed() && this->start_handshake_button.value_or (false))
		start_handshake();

	try {
		if (_handshake_response_changed.value_changed() && this->handshake_response.valid())
			if (_next_session_candidate->waiting_for_handshake_response())
				_next_session_candidate->set_handshake_response (nu::to_blob (*this->handshake_response));
	}
	catch (...)
	{
		logger() << std::format ("Exception when handling handshake response: {}\n", nu::describe_exception (std::current_exception()));
	}
}


void
MasterSecureChannel::shift_sessions()
{
	if (_next_session_candidate)
	{
		_previous_session = std::move (_active_session);
		_active_session = std::move (_next_session_candidate);
		this->handshake_request = xf::nil;
		_active_session->set_activated();
	}
}


SlaveSecureChannel::Session::Session (Blob const& handshake_request,
									  CryptoParams const& params,
									  HandshakeSlave::KeyCheckFunctions const key_check_callbacks):
	SecureChannel::Session ("S", _id_generator)
{
	auto handshake_slave = HandshakeSlave (global::transceiver_rnd, {
		.master_signature_key = params.master_signature_key,
		.slave_signature_key = params.slave_signature_key,
		.hmac_size = params.hmac_size,
		.max_time_difference = params.max_time_difference,
	}, key_check_callbacks);
	auto const response_and_key = handshake_slave.generate_handshake_blob_and_key (handshake_request, nu::utc_now());
	_handshake_response = response_and_key.handshake_response;
	_transmitter.emplace (global::transceiver_rnd, Transmitter::Params {
		.ephemeral_session_key = *response_and_key.ephemeral_key,
		.authentication_secret = params.authentication_secret,
		.data_encryption_secret = params.data_encryption_secret,
		.seq_num_encryption_secret = params.seq_num_encryption_secret,
		.hmac_size = params.hmac_size,
		// This ensures actual keys are different for both directions:
		.hkdf_user_info = global::kSlaveToMaster,
	});
	_receiver.emplace (Receiver::Params {
		.ephemeral_session_key = *response_and_key.ephemeral_key,
		.authentication_secret = params.authentication_secret,
		.data_encryption_secret = params.data_encryption_secret,
		.seq_num_encryption_secret = params.seq_num_encryption_secret,
		.hmac_size = params.hmac_size,
		// This ensures actual keys are different for both directions:
		.hkdf_user_info = global::kMasterToSlave,
	});
}


SlaveSecureChannel::SlaveSecureChannel (xf::ProcessingLoop& loop,
										CryptoParams const& params,
										HandshakeSlave::KeyCheckFunctions const key_check_functions,
										nu::Logger const& logger,
										std::string_view const instance):
	SecureChannel (Role::Slave, Transport::ciphertext_expansion (params.hmac_size), logger.with_context (std::string (kLoggerScope) + "#" + instance)),
	Module (loop, "SlaveSecureChannel"),
	_crypto_params (params),
	_key_check_functions (key_check_functions)
{ }


std::optional<Blob>
SlaveSecureChannel::tx_key_hash() const
{
	if (_active_session)
		return _active_session->tx_key_hash();
	else
		return std::nullopt;
}


std::optional<Blob>
SlaveSecureChannel::rx_key_hash() const
{
	if (_active_session)
		return _active_session->rx_key_hash();
	else
		return std::nullopt;
}


void
SlaveSecureChannel::process (Cycle const&)
{
	try {
		if (_handshake_request_changed.value_changed() && this->handshake_request.valid())
		{
			_next_session_candidate = std::make_unique<Session> (nu::to_blob (*this->handshake_request), _crypto_params, _key_check_functions);
			this->handshake_response = nu::to_string (*_next_session_candidate->handshake_response());
		}
	}
	catch (...)
	{
		logger() << std::format ("Exception when handling handshake request: {}\n", nu::describe_exception (std::current_exception()));
	}
}


void
SlaveSecureChannel::shift_sessions()
{
	if (_next_session_candidate)
	{
		_active_session = std::move (_next_session_candidate);
		this->handshake_response = xf::nil;
	}
}

} // namespace xf::crypto::xle
