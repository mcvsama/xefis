/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Xefis:
#include <xefis/core/processing_loop.h>
#include <xefis/modules/comm/xle_transceiver.h>
#include <xefis/support/crypto/xle/handshake.h>
#include <xefis/test/test_processing_loop.h>

// Neutrino:
#include <neutrino/test/auto_test.h>
#include <neutrino/exception_support.h>
#include <neutrino/string.h>

// Boost:
#include <boost/random.hpp>

// Standard:
#include <cstddef>
#include <format>


namespace xf::xle_transceiver_test {

namespace test_asserts = nu::test_asserts;
namespace xle = xf::crypto::xle;

auto constinit kKeysDebugging = false;
auto constinit crypto_params = xle::Transceiver::CryptoParams {
	.master_signature_key		= { 0x00, 0x01, 0x02, 0x03 },
	.slave_signature_key		= { 0x0c, 0x0d, 0x0e, 0x0f },
	.authentication_secret		= { 0x01 },
	.data_encryption_secret		= { 0x02 },
	.seq_num_encryption_secret	= { 0x03 },
	.hmac_size					= 16,
	.max_time_difference		= 60_s,
};

enum class KeysMode
{
	Unavailable,
	Available,
	FromPreviousSession,
};


struct Expectations
{
	KeysMode					keys_mode;
	bool						master_connecting;
	bool						master_connected;
	bool						master_offers_handshake_request;
	bool						slave_connecting;
	bool						slave_connected;
	bool						slave_offers_handshake_response;
	std::shared_future<void>*	session_prepared_future = nullptr;
	bool						session_prepared_future_is_ready = false;
	std::shared_future<void>*	session_activated_future = nullptr;
	bool						session_activated_future_is_ready = false;
	std::shared_future<void>*	previous_session_activated_future = nullptr;
	bool						previous_session_activated_future_throws = false; // In case of testing aborted handshake (new handshake before previous one is finished).
};


// Can't use `friend a::b::function()`, but `friend class a::b::someclass` is allowed.
// This testing code needs to access private members of tested objects.
struct AutoTestT1
{
	static void
	auto_test_t1();
};


void
AutoTestT1::auto_test_t1()
{
	auto constexpr kMethodCallStart = 0;
	auto constexpr kButtonPressStart = 1;

	auto constexpr kSessions = 8;

	// Testing two start modes: by calling MasterTransceiver::start_handshake() and by emulating
	// pressing a button with property MasterTransceiver::start_handshake_button.
	for (int start_mode: { kMethodCallStart, kButtonPressStart })
	{
		auto loop = TestProcessingLoop (0.01_s);
		// In reverse order to make sure slave gets handled first, which means
		// it will cause master to be processed first, since slave depends
		// on master. So in the end master.process() will be called first,
		// then slave.process().
		auto slave = xle::SlaveTransceiver (loop, crypto_params, {}, TestProcessingLoop::logger);
		auto master = xle::MasterTransceiver (loop, crypto_params, TestProcessingLoop::logger);

		slave.handshake_request << master.handshake_request;
		master.handshake_response << slave.handshake_response;

		std::string start_mode_prefix = (start_mode == kMethodCallStart)
			? "start by start_handshake()"
			: "start by pressing button";

		// History of hashed keys used for different sessions:
		auto master_tx_session_keys = std::vector<Blob> (kSessions);
		auto master_rx_session_keys = std::vector<Blob> (kSessions);
		auto slave_tx_session_keys = std::vector<Blob> (kSessions);
		auto slave_rx_session_keys = std::vector<Blob> (kSessions);
		auto finished_handshakes = 0u;

		auto const verify_expectations = [&] (std::string p, Expectations const& expectations) -> void {
			p += ": ";

			switch (expectations.keys_mode)
			{
				case KeysMode::Unavailable:
					test_asserts::verify (p + "master TX key is undefined", !master.tx_key_hash());
					test_asserts::verify (p + "master RX key is undefined", !master.rx_key_hash());
					test_asserts::verify (p + "slave TX key is undefined", !slave.tx_key_hash());
					test_asserts::verify (p + "slave RX key is undefined", !slave.rx_key_hash());
					break;

				case KeysMode::Available:
					test_asserts::verify (p + "master TX key is available", master.tx_key_hash().value_or (Blob()).size() > 0);
					test_asserts::verify (p + "master RX key is available", master.rx_key_hash().value_or (Blob()).size() > 0);
					test_asserts::verify (p + "slave TX key is available", slave.tx_key_hash().value_or (Blob()).size() > 0);
					test_asserts::verify (p + "slave RX key is available", slave.rx_key_hash().value_or (Blob()).size() > 0);

					// Make sure keys are not the same as in previous session:
					if (finished_handshakes > 0)
					{
						test_asserts::verify (p + "master TX key is different than in previous session",
											  master.tx_key_hash() != master_tx_session_keys[finished_handshakes - 1]);
						test_asserts::verify (p + "master RX key is different than in previous session",
											  master.rx_key_hash() != master_rx_session_keys[finished_handshakes - 1]);
						test_asserts::verify (p + "slave TX key is different than in previous session",
											  slave.tx_key_hash() != slave_tx_session_keys[finished_handshakes - 1]);
						test_asserts::verify (p + "slave RX key is different than in previous session",
											  slave.rx_key_hash() != slave_rx_session_keys[finished_handshakes - 1]);
					}

					// Make sure TX and RX keys are different and they match between master and slave:
					test_asserts::verify (p + "master.rx_key == slave.tx_key", master.rx_key_hash() == slave.tx_key_hash());
					test_asserts::verify (p + "master.tx_key == slave.rx_key", master.tx_key_hash() == slave.rx_key_hash());
					test_asserts::verify (p + "RX and TX keys are different", master.rx_key_hash() != master.tx_key_hash());
					break;

				case KeysMode::FromPreviousSession:
					test_asserts::verify (p + "master TX key is still the one from previous session",
										  master.tx_key_hash() == master_tx_session_keys[finished_handshakes - 1]);
					test_asserts::verify (p + "master RX key is still the one from previous session",
										  master.rx_key_hash() == master_rx_session_keys[finished_handshakes - 1]);
					test_asserts::verify (p + "slave TX key is still the one from previous session",
										  slave.tx_key_hash() == slave_tx_session_keys[finished_handshakes - 1]);
					test_asserts::verify (p + "slave RX key is still the one from previous session",
										  slave.rx_key_hash() == slave_rx_session_keys[finished_handshakes - 1]);
					break;
			}

			if (expectations.master_connecting)
				test_asserts::verify (p + "master says 'connecting'", master.connecting());
			else
				test_asserts::verify (p + "master says 'not connecting'", !master.connecting());

			if (expectations.slave_connecting)
				test_asserts::verify (p + "slave says 'connecting'", slave.connecting());
			else
				test_asserts::verify (p + "slave says 'not connecting'", !slave.connecting());

			if (expectations.master_connected)
				test_asserts::verify (p + "master says 'connected'", master.connected());
			else
				test_asserts::verify (p + "master says 'not connected'", !master.connected());

			if (expectations.slave_connected)
				test_asserts::verify (p + "slave says 'connected'", slave.connected());
			else
				test_asserts::verify (p + "slave says 'not connected'", !slave.connected());

			if (expectations.master_offers_handshake_request)
				test_asserts::verify (p + "master offers handshake request", master.handshake_request && master.handshake_request->size() > 0);
			else
				test_asserts::verify (p + "master doesn't offer handshake request", master.handshake_request.is_nil());

			if (expectations.slave_offers_handshake_response)
				test_asserts::verify (p + "slave offers handshake response", slave.handshake_response && slave.handshake_response->size() > 0);
			else
				test_asserts::verify (p + "slave doesn't offer handshake response", slave.handshake_response.is_nil());

			if (expectations.session_prepared_future)
			{
				test_asserts::verify (p + "session_prepared_future is valid", expectations.session_prepared_future->valid());

				if (expectations.session_prepared_future_is_ready)
					test_asserts::verify (p + "session_prepared_future is ready", nu::ready (*expectations.session_prepared_future));
				else
					test_asserts::verify (p + "session_prepared_future is not ready", !nu::ready (*expectations.session_prepared_future));

				if (nu::ready (*expectations.session_prepared_future))
				{
					bool got_generic_handshake_exception = false;

					try {
						expectations.session_prepared_future->get();
					}
					catch (...)
					{
						got_generic_handshake_exception = true;
					}

					test_asserts::verify (p + "session_prepared_future is fulfilled", !got_generic_handshake_exception);
				}
			}

			if (expectations.session_activated_future)
			{
				test_asserts::verify (p + "session_activated_future is valid", expectations.session_activated_future->valid());

				if (expectations.session_activated_future_is_ready)
					test_asserts::verify (p + "session_activated_future is ready", nu::ready (*expectations.session_activated_future));
				else
					test_asserts::verify (p + "session_activated_future is not ready", !nu::ready (*expectations.session_activated_future));

				if (nu::ready (*expectations.session_activated_future))
				{
					bool got_generic_handshake_exception = false;

					try {
						expectations.session_activated_future->get();
					}
					catch (...)
					{
						got_generic_handshake_exception = true;
					}

					test_asserts::verify (p + "session_activated_future is fulfilled", !got_generic_handshake_exception);
				}
			}

			if (expectations.previous_session_activated_future)
			{
				test_asserts::verify (p + "previous_session_activated_future is valid", expectations.previous_session_activated_future->valid());
				test_asserts::verify (p + "previous_session_activated_future is ready", nu::ready (*expectations.previous_session_activated_future));

				bool got_generic_handshake_exception = false;
				bool got_handshake_aborted_exception = false;

				try {
					expectations.previous_session_activated_future->get();
				}
				catch (xle::MasterTransceiver::HandshakeAborted const&)
				{
					got_handshake_aborted_exception = true;
				}
				catch (...)
				{
					got_generic_handshake_exception = true;
				}

				if (expectations.previous_session_activated_future_throws)
				{
					test_asserts::verify (p + "previous_session_activated_future throws HandshakeAborted", got_handshake_aborted_exception);
					test_asserts::verify (p + "previous_session_activated_future only throws HandshakeAborted", !got_generic_handshake_exception);
				}
				else
				{
					test_asserts::verify (p + "previous_session_activated_future is fulfilled (1)", !got_generic_handshake_exception);
					test_asserts::verify (p + "previous_session_activated_future is fulfilled (2)", !got_handshake_aborted_exception);
				}
			}
		};

		auto previous_handshake_was_interrupted_at_master = false;

		std::shared_future<void> session_prepared;
		std::shared_future<void>* session_prepared_ptr = (start_mode == kMethodCallStart)
			? &session_prepared
			: nullptr;
		std::shared_future<void> session_activated;
		std::shared_future<void>* session_activated_ptr = (start_mode == kMethodCallStart)
			? &session_activated
			: nullptr;

		for (size_t session_number = 0; session_number < kSessions; ++session_number)
		{
			auto const prefix = std::format ("{}, session {}: ", start_mode_prefix, session_number);

			auto const test_communication = [&] (std::string p, std::optional<int> session_number = std::nullopt) {
				if (kKeysDebugging)
				{
					auto const print_m_session_keys = [&] (std::string_view const prefix, auto* session) {
						if (session)
						{
							std::cout << std::format ("    {} TX == {} {}\n", prefix, session->id(), session->tx_key_hash() ? nu::to_hex_string (*session->tx_key_hash()) : "-");
							std::cout << std::format ("    {} RX == {} {}\n", prefix, session->id(), session->rx_key_hash() ? nu::to_hex_string (*session->rx_key_hash()) : "-");
						}
						else
						{
							std::cout << std::format ("    {} TX == - -\n", prefix);
							std::cout << std::format ("    {} RX == - -\n", prefix);
						}
					};

					auto const print_s_session_keys = [&] (std::string_view const prefix, auto* session) {
						if (session)
						{
							std::cout << std::format ("    {} TX == {} {}\n", prefix, session->id(), nu::to_hex_string (session->tx_key_hash()));
							std::cout << std::format ("    {} RX == {} {}\n", prefix, session->id(), nu::to_hex_string (session->rx_key_hash()));
						}
						else
						{
							std::cout << std::format ("    {} TX == - -\n", prefix);
							std::cout << std::format ("    {} RX == - -\n", prefix);
						}
					};

					print_m_session_keys ("master.previous_session", master.previous_session());
					print_m_session_keys ("master.active_session", master.active_session());
					print_m_session_keys ("master.next_session", master.next_session_candidate());
					print_s_session_keys ("slave.active_session", slave.active_session());
					print_s_session_keys ("slave.next_session", slave.next_session_candidate());
				}

				p += ": ";

				// If session_number is provided, only test communication if it's > 0, since then we can
				// assume that previous session is already established:
				if (session_number.value_or (1) > 0)
				{
					auto const p1 = Blob { 0x00, 0x01, 0x02, 0x03 };
					auto const p2 = Blob { 0x09, 0x08, 0x07, 0x06 };

					// Both ends switch next sessions to active
					// sessions upon receiving correctly encrypted packets.
					// If slave tries to encrypt first when master
					// just changed session (and slave still has old session),
					// it will use old session. So this order of events tests that
					// decrypting with old session by master is still possible.

					// Master can't decrypt with active session S1, switches to next session S2:
					auto const e1 = master.encrypt_packet (p1);
					// Slave encrypts with session S1, since it has switched yet, because it has
					// not decrypted anything with S2 keys:
					auto const e2 = slave.encrypt_packet (p2);
					// Slave decrypts with S2 since it can't decrypt with S1:
					auto const d1 = slave.decrypt_packet (e1);
					// Master receives packet encrypted with session S1, but its active session
					// is already S2. It's forced to use previous session keys as a fallback:
					auto const d2 = master.decrypt_packet (e2);

					test_asserts::verify (prefix + p + "encryption master → slave works correctly", p1 == d1);
					test_asserts::verify (prefix + p + "encryption slave → master works correctly", p2 == d2);
				}
			};

			if (kKeysDebugging)
				std::cout << std::format ("-- Session {} --\n", session_number);

			if (session_number == 0)
			{
				verify_expectations (prefix + "step 0", {
					.keys_mode = KeysMode::Unavailable,
					.master_connecting = false,
					.master_connected = false,
					.master_offers_handshake_request = false,
					.slave_connecting = false,
					.slave_connected = false,
					.slave_offers_handshake_response = false,
				});
			}
			else
			{
				verify_expectations (prefix + "step 0", {
					.keys_mode = KeysMode::FromPreviousSession,
					.master_connecting = previous_handshake_was_interrupted_at_master,
					.master_connected = true,
					.master_offers_handshake_request = previous_handshake_was_interrupted_at_master,
					.slave_connecting = previous_handshake_was_interrupted_at_master,
					.slave_connected = true,
					.slave_offers_handshake_response = previous_handshake_was_interrupted_at_master,
				});
			}

			// Before new session is created, communication must still work over the current session:
			test_communication ("before starting new handshake", session_number);

			if (start_mode == kMethodCallStart)
			{
				auto previous_session_prepared = session_prepared;
				auto previous_session_activated = session_activated;
				auto started = master.start_handshake();
				session_prepared = started.session_prepared;
				session_activated = started.session_activated;

				test_communication ("just after calling start_handshake()", session_number);

				if (session_number == 0)
				{
					verify_expectations (prefix + "step 1", {
						.keys_mode = KeysMode::Unavailable,
						.master_connecting = true,
						.master_connected = false,
						.master_offers_handshake_request = true,
						.slave_connecting = false,
						.slave_connected = false,
						.slave_offers_handshake_response = false,
						.session_prepared_future = session_prepared_ptr,
						.session_prepared_future_is_ready = false,
						.session_activated_future = session_activated_ptr,
						.session_activated_future_is_ready = false,
					});
				}
				else
				{
					verify_expectations (prefix + "step 1", {
						.keys_mode = KeysMode::FromPreviousSession,
						.master_connecting = true,
						.master_connected = true,
						.master_offers_handshake_request = true,
						.slave_connecting = previous_handshake_was_interrupted_at_master,
						.slave_connected = true,
						.slave_offers_handshake_response = previous_handshake_was_interrupted_at_master,
						.session_prepared_future = session_prepared_ptr,
						.session_prepared_future_is_ready = false,
						.session_activated_future = session_activated_ptr,
						.session_activated_future_is_ready = false,
						.previous_session_activated_future = &previous_session_prepared,
						.previous_session_activated_future_throws = previous_handshake_was_interrupted_at_master,
					});
				}

				loop.next_cycle();

				// Don't finish this handshake, start new handshake early:
				if (session_number == 3)
				{
					previous_handshake_was_interrupted_at_master = true;
					continue;
				}

				loop.next_cycle();
			}
			else
			{
				test_communication ("just before pressing button", session_number);
				master.start_handshake_button << true;
				test_communication ("just after pressing button", session_number);
				loop.next_cycle();
				test_communication ("after pressing button and 1 cycle", session_number);
				master.start_handshake_button << false;
				loop.next_cycle();
			}

			if (session_number == 0)
			{
				verify_expectations (prefix + "step 2", {
					.keys_mode = KeysMode::Unavailable,
					.master_connecting = true,
					.master_connected = false,
					.master_offers_handshake_request = true,
					.slave_connecting = true,
					.slave_connected = false,
					.slave_offers_handshake_response = true,
					.session_prepared_future = session_prepared_ptr,
					.session_prepared_future_is_ready = true,
					.session_activated_future = session_activated_ptr,
					.session_activated_future_is_ready = false,
				});
			}
			else
			{
				verify_expectations (prefix + "step 2", {
					.keys_mode = KeysMode::FromPreviousSession,
					.master_connecting = true,
					.master_connected = true,
					.master_offers_handshake_request = true,
					.slave_connecting = true,
					.slave_connected = true,
					.slave_offers_handshake_response = true,
					.session_prepared_future = session_prepared_ptr,
					.session_prepared_future_is_ready = true,
					.session_activated_future = session_activated_ptr,
					.session_activated_future_is_ready = false,
				});
			}

			test_communication ("first communication on new session");

			if (session_number == 0)
			{
				verify_expectations (prefix + "step 3", {
					.keys_mode = KeysMode::Available,
					.master_connecting = false,
					.master_connected = true,
					.master_offers_handshake_request = false,
					.slave_connecting = false,
					.slave_connected = true,
					.slave_offers_handshake_response = false,
					.session_prepared_future = session_prepared_ptr,
					.session_prepared_future_is_ready = true,
					.session_activated_future = session_activated_ptr,
					.session_activated_future_is_ready = true,
				});
			}
			else
			{
				verify_expectations (prefix + "step 3", {
					.keys_mode = KeysMode::Available,
					.master_connecting = false,
					.master_connected = true,
					.master_offers_handshake_request = false,
					.slave_connecting = false,
					.slave_connected = true,
					.slave_offers_handshake_response = false,
					.session_prepared_future = session_prepared_ptr,
					.session_prepared_future_is_ready = true,
					.session_activated_future = session_activated_ptr,
					.session_activated_future_is_ready = true,
				});
			}

			master_tx_session_keys[finished_handshakes] = *master.tx_key_hash();
			master_rx_session_keys[finished_handshakes] = *master.rx_key_hash();
			slave_tx_session_keys[finished_handshakes] = *slave.tx_key_hash();
			slave_rx_session_keys[finished_handshakes] = *slave.rx_key_hash();
			++finished_handshakes;
			previous_handshake_was_interrupted_at_master = false;
		}

		master.disconnect();
		slave.disconnect();

		verify_expectations ("after calling disconnect()", {
			.keys_mode = KeysMode::Unavailable,
			.master_connecting = false,
			.master_connected = false,
			.master_offers_handshake_request = false,
			.slave_connecting = false,
			.slave_connected = false,
			.slave_offers_handshake_response = false,
		});
	}
}


void
auto_test_t2()
{
	auto rnd = boost::random::random_device();
	auto uniform_distribution = boost::random::uniform_int_distribution<size_t>();

	auto make_random_loss = [&rnd, &uniform_distribution] (std::string_view const message) {
		return [&rnd, &uniform_distribution, message] (std::optional<std::string> input) -> std::optional<std::string> {
			if (input && !input->empty())
			{
				// Modify with probability 80%:
				if (uniform_distribution (rnd) % 10 < 9)
				{
					auto const byte_to_alter = uniform_distribution (rnd) % input->size();
					input->at (byte_to_alter) ^= 0x55;
				}
			}

			return input;
		};
	};

	auto loop = TestProcessingLoop (0.01_s);
	auto slave = xle::SlaveTransceiver (loop, crypto_params, {}, TestProcessingLoop::logger);
	auto master = xle::MasterTransceiver (loop, crypto_params, TestProcessingLoop::logger);

	slave.handshake_request << std::function (make_random_loss ("master → slave")) << master.handshake_request;
	master.handshake_response << std::function (make_random_loss ("slave → master")) << slave.handshake_response;

	auto const test_communication = [&] (std::string p, std::optional<bool> expected_to_fail_opt = std::nullopt) {
		static auto const p1 = nu::to_blob ("(master → slave) message");
		static auto const p2 = nu::to_blob ("(slave → master) message");

		p += ": ";

		std::string exception_message;
		Blob e1, e2, d1, d2;

		try {
			// Master can't decrypt with active session S1, switches to next session S2:
			e1 = master.encrypt_packet (p1);
			// Slave encrypts with session S1, since it has switched yet, because it has
			// not decrypted anything with S2 keys:
			e2 = slave.encrypt_packet (p2);
			// Slave decrypts with S2 since it can't decrypt with S1:
			d1 = slave.decrypt_packet (e1);
			// Master receives packet encrypted with session S1, but its active session
			// is already S2. It's forced to use previous session keys as a fallback:
			d2 = master.decrypt_packet (e2);
		}
		catch (...)
		{
			exception_message = nu::describe_exception (std::current_exception());
		}

		if (expected_to_fail_opt)
		{
			if (*expected_to_fail_opt)
				test_asserts::verify (p + "encryption/decryption throws", !exception_message.empty());
			else
			{
				test_asserts::verify (std::format ("{}encryption/decryption works (exception: {})", p, exception_message), exception_message.empty());
				test_asserts::verify (p + "encryption master → slave works correctly", p1 == d1);
				test_asserts::verify (p + "encryption slave → master works correctly", p2 == d2);
			}
		}
	};

	for (size_t session_number = 0; session_number < 10; ++session_number)
	{
		auto const [session_prepared, session_activated] = master.start_handshake();

		if (session_number == 0)
			test_communication ("just after calling start_handshake()", true);

		do {
			loop.next_cycles (2);
		} while (!nu::ready (session_prepared));

		do {
			loop.next_cycles (2);
			test_communication ("just after calling start_handshake()");
		} while (!nu::ready (session_activated));

		session_prepared.get();
	}
}


nu::AutoTest t1 ("Xefis Lossy Encryption/Protocol: handshaking gives correct keys", AutoTestT1::auto_test_t1);
nu::AutoTest t2 ("Xefis Lossy Encryption/Protocol: handshaking eventually works even on lossy channel", auto_test_t2);

} // namespace xf::xle_transceiver_test

