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

#ifndef XEFIS__MODULES__COMM__XLE_TRANSCEIVER_H__INCLUDED
#define XEFIS__MODULES__COMM__XLE_TRANSCEIVER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/crypto/xle/handshake.h>
#include <xefis/support/crypto/xle/transport.h>
#include <xefis/support/sockets/socket_value_changed.h>

// Neutrino:
#include <neutrino/crypto/secure.h>
#include <neutrino/noncopyable.h>

// Qt:
#include <QtCore/QTimer>

// Standard:
#include <cstddef>
#include <functional>


namespace xf::xle_transceiver_test {

struct AutoTestT1;

} // namespace xf::test


namespace xf::crypto::xle {

namespace si = neutrino::si;

enum class Role {
	Master,
	Slave,
};


/**
 * For debugging and test logs.
 */
class WithIDs
{
  public:
	WithIDs (std::string_view const prefix, size_t& id_generator):
		_id (std::format ("{}-{}", prefix, ++id_generator))
	{ }

	[[nodiscard]]
	std::string
	id() const
		{ return _id; }

  private:
	std::string _id;
};


class Transceiver
{
  public:
	/**
	 * Encryption parameters to use.
	 */
	struct CryptoParams
	{
		// Size of each key should be CryptoPP::AES::MAX_KEYLENGTH or even more.
		// At commit time it's 32 or more bytes.

		// Key used to sign master handshakes:
		Blob		master_signature_key;
		// Key used to sign slave handshakes:
		Blob		slave_signature_key;
		// Keys for encryption/signing various parts of packets:
		Blob		authentication_secret;
		Blob		data_encryption_secret;
		Blob		seq_num_encryption_secret;
		// HMAC length, 12 should be a minimum:
		size_t		hmac_size				{ 12 };
		si::Time	max_time_difference		{ 60_s };
	};

  protected:
	class Session: public WithIDs
	{
	  public:
		using WithIDs::WithIDs;

		// Dtor
		virtual
		~Session() = default;

		[[nodiscard]]
		virtual bool
		connected() const = 0;

		[[nodiscard]]
		virtual Transmitter const&
		transmitter() const = 0;

		[[nodiscard]]
		virtual Blob
		encrypt_packet (BlobView) = 0;

		[[nodiscard]]
		virtual Blob
		decrypt_packet (BlobView, std::optional<Transport::SequenceNumber> const maximum_allowed_sequence_number = std::nullopt) = 0;
	};

  public:
	/**
	 * Ctor
	 *
	 * \param	ciphertext_expansion
	 *			How many bytes the encrypted packet will be bigger than the original unencrypted one.
	 */
	explicit
	Transceiver (Role, size_t ciphertext_expansion, xf::Logger const&);

	// Dtor
	virtual
	~Transceiver() = default;

	/**
	 * Return true if protocol is currently in the Connected state, which means
	 * it can encrypt/decrypt packets.
	 */
	[[nodiscard]]
	bool
	connected() const noexcept
		{ return !!active_session(); }

	/**
	 * Return true if protocol is currently in the process of handshaking
	 * a new session.
	 */
	[[nodiscard]]
	bool
	connecting() const noexcept
		{ return !!next_session_candidate(); }

	/**
	 * Return true if the transceiver is ready to encrypt/decrypt packets.
	 */
	[[nodiscard]]
	bool
	ready() const;

	/**
	 * Return how much larger the resulting packet will be compared to plain text.
	 * This function can be called only if connected() returns true.
	 */
	[[nodiscard]]
	size_t
	ciphertext_expansion() const
		{ return _ciphertext_expansion; }

	/**
	 * Encrypt packet.
	 *
	 * \throws std::logic_error when calculated HMAC size doesn't fit
	 *		   requirements.
	 * \throws Exception when transceiver is not connected.
	 */
	[[nodiscard]]
	virtual Blob
	encrypt_packet (BlobView);

	/**
	 * Decrypt packet.
	 *
	 * \throws DecryptionFailure on various occassions.
	 * \throws Exception when transceiver is not connected.
	 */
	[[nodiscard]]
	virtual Blob
	decrypt_packet (BlobView, std::optional<Transport::SequenceNumber> const maximum_allowed_sequence_number = std::nullopt);

  protected:
	/**
	 * Return previous session which was previously an active session before
	 * shift_session().
	 */
	[[nodiscard]]
	virtual Session*
	previous_session() = 0;

	/**
	 * Return previous session which was previously an active session before
	 * shift_session().
	 */
	[[nodiscard]]
	virtual Session const*
	previous_session() const = 0;

	/**
	 * Return session that is currently used to encrypt and decrypt packets.
	 */
	[[nodiscard]]
	virtual Session*
	active_session() = 0;

	/**
	 * Return session that is currently used to encrypt and decrypt packets.
	 */
	[[nodiscard]]
	virtual Session const*
	active_session() const = 0;

	/**
	 * Return session that is being prepared as the next one to become active.
	 */
	[[nodiscard]]
	virtual Session*
	next_session_candidate() = 0;

	/**
	 * Return session that is being prepared as the next one to become active.
	 */
	[[nodiscard]]
	virtual Session const*
	next_session_candidate() const = 0;

	/**
	 * Make next session candidate the new active session.
	 */
	virtual void
	shift_sessions() = 0;

	/**
	 * Destroy previous session to save resources.
	 */
	virtual void
	get_rid_of_previous_session()
	{ }

  protected:
	std::string
	role_name() const;

	xf::Logger const&
	logger() const noexcept
		{ return _logger; }

  private:
	Role const	_role;
	xf::Logger	_logger;
	size_t		_ciphertext_expansion;
};


/**
 * A class used on the side that initiates communication.
 * A single DHE is used to establish long session key from which
 * two encryption keys are derived (one for each direction of
 * communication).
 */
class MasterTransceiver:
	public Transceiver,
	public xf::Module
{
	friend class xf::xle_transceiver_test::AutoTestT1;

  public:
	/*
	 * Input
	 */

	xf::ModuleIn<bool>			start_handshake_button	{ this, "start_handshake_button" };
	xf::ModuleIn<std::string>	handshake_response		{ this, "handshake_response" };

	/*
	 * Output
	 */

	// It's non-nil when offering a handshake, becomes nil after the handshake is complete.
	xf::ModuleOut<std::string>	handshake_request		{ this, "handshake_request" };

  public:
	enum AbortReason
	{
		NewHandshakeStarted,
		Deleted,
	};

	/**
	 * Exception class used in future returned by start_handshake().
	 */
	struct HandshakeAborted
	{
		AbortReason reason;
	};

	struct StartHandshakeResult
	{
		// The session_prepared is fulfilled when the session is prepared,
		// but not communication happened yet, so it didn't yet become an active session.
		// Gets rejected when session handshake is abandoned or a new start_handshake()
		// call is made.
		std::shared_future<void>	session_prepared;

		// session_activated is fulfilled when the prepared session becomes active, after
		// the Transceiver has received correctly encrypted packets from the remote end.
		// Gets rejected when session handshake is abandoned or a new start_handshake()
		// call is made.
		std::shared_future<void>	session_activated;
	};

  private:
	static constexpr char kLoggerScope[] = "mod::MasterTransceiver";

    /**
	 * An established communication session.
     */
	class Session:
		public Transceiver::Session,
		public neutrino::Noncopyable
	{
		static inline size_t _id_generator = 0;

	  public:
		/**
		 * HandshakeRequested state.
		 * Handshake has been requested to be sent, and it will be requested periodically
		 * until connection is finalized, that is handshake response is received and encryption
		 * key gets calculated.
		 */
		struct HandshakeRequested
		{
			HandshakeMaster	handshake_master;
			Blob			handshake_request;

			// Ctor
			explicit
			HandshakeRequested (CryptoParams const&);
		};

		/**
		 * Connected state.
		 * Encryption keys are available and encryption packets can be sent/received.
		 */
		struct Connected
		{
			Transmitter			transmitter;
			Receiver			receiver;

			// Ctor
			explicit
			Connected (Secure<Blob> const& ephemeral_key, CryptoParams const& params);
		};

		// Ctor
		explicit
		Session (CryptoParams const&);

		// Dtor
		~Session();

		/**
		 * Return the handshake request blob to be sent to the SlaveTransceiver.
		 */
		[[nodiscard]]
		Blob const&
		handshake_request() const;

		/**
		 * Return true if session is awaiting for handshake response
		 * to be set with set_handshake_response().
		 */
		[[nodiscard]]
		bool
		waiting_for_handshake_response() const noexcept
			{ return std::holds_alternative<HandshakeRequested> (_state); }

		/**
		 * Use handshake response obtained from SlaveTransceiver.
		 */
		void
		set_handshake_response (Blob const&);

		/**
		 * Called when this session becomes the main active session.
		 */
		void
		set_activated()
			{ _session_activated_promise.set_value(); }

		/**
		 * Call this when new handshake is created and this session gets
		 * abandoned.
		 */
		void
		abort (AbortReason);

		/**
		 * Return a std::future that gets resolved when a handshake is completed
		 * but session is not confirmed as working, so it still needs to exchange
		 * encrypted data.
		 */
		[[nodiscard]]
		std::shared_future<void>
		session_prepared()
			{ return _session_prepared_future; }

		[[nodiscard]]
		std::shared_future<void>
		session_activated()
			{ return _session_activated_future; }

		[[nodiscard]]
		std::optional<Blob>
		tx_key_hash() const;

		[[nodiscard]]
		std::optional<Blob>
		rx_key_hash() const;

		// Session API
		[[nodiscard]]
		bool
		connected() const override
			{ return std::holds_alternative<Connected> (_state); }

		[[nodiscard]]
		Transmitter&
		transmitter();

		// Session API
		[[nodiscard]]
		Transmitter const&
		transmitter() const override
			{ return const_cast<Session*> (this)->transmitter(); }

		[[nodiscard]]
		Receiver&
		receiver();

		// Session API
		[[nodiscard]]
		Blob
		encrypt_packet (BlobView const packet)
			{ return this->transmitter().encrypt_packet (packet); }

		// Session API
		[[nodiscard]]
		Blob
		decrypt_packet (BlobView const packet, std::optional<Transport::SequenceNumber> const maximum_allowed_sequence_number = std::nullopt)
			{ return this->receiver().decrypt_packet (packet, maximum_allowed_sequence_number); }

	  private:
		CryptoParams								_crypto_params;
		std::variant<HandshakeRequested, Connected>	_state;
		std::shared_future<void>					_session_prepared_future;
		std::promise<void>							_session_prepared_promise;
		std::shared_future<void>					_session_activated_future;
		std::promise<void>							_session_activated_promise;
	};

  public:
	// Ctor
	explicit
	MasterTransceiver (xf::ProcessingLoop&, CryptoParams const&, xf::Logger const&, std::string_view const instance = {});

	/**
	 * Perform a handshake.
	 * Same effect can be achieved by using start_handshake_button input socket.
	 *
	 */
	StartHandshakeResult
	start_handshake();

	/**
	 * Disconnect active connection if connected.
	 */
	void
	disconnect() noexcept
		{ _active_session.reset(); }

	[[nodiscard]]
	std::optional<Blob>
	tx_key_hash() const;

	[[nodiscard]]
	std::optional<Blob>
	rx_key_hash() const;

  protected:
	// Module API
	void
	process (Cycle const&);

	// Transceiver API
	[[nodiscard]]
	Session*
	previous_session() override
		{ return _previous_session.get(); }

	// Transceiver API
	[[nodiscard]]
	Session const*
	previous_session() const override
		{ return _previous_session.get(); }

	// Transceiver API
	[[nodiscard]]
	Session*
	active_session() override
		{ return _active_session.get(); }

	// Transceiver API
	[[nodiscard]]
	Session const*
	active_session() const override
		{ return _active_session.get(); }

	// Transceiver API
	[[nodiscard]]
	Session*
	next_session_candidate() override
		{ return _next_session_candidate.get(); }

	// Transceiver API
	[[nodiscard]]
	Session const*
	next_session_candidate() const override
		{ return _next_session_candidate.get(); }

	// Transceiver API
	void
	shift_sessions() override;

	// Transceiver API
	void
	get_rid_of_previous_session() override
		{ _previous_session.reset(); }

  private:
	CryptoParams						_crypto_params;
	xf::SocketValueChanged<bool>		_start_handshake_button_changed { start_handshake_button };
	xf::SocketValueChanged<std::string>	_handshake_response_changed     { handshake_response };
	// Active session means connected and confirmed correct encryption+authentication:
	std::unique_ptr<Session>			_previous_session;
	std::unique_ptr<Session>			_active_session;
	std::unique_ptr<Session>			_next_session_candidate;
};


/**
 * A class used on the side that only receives and handles handshake requests.
 */
class SlaveTransceiver:
	public Transceiver,
	public xf::Module
{
	friend class xf::xle_transceiver_test::AutoTestT1;

  public:
	xf::ModuleIn<std::string>	handshake_request		{ this, "handshake_request" };

	xf::ModuleOut<uint64_t>		num_received_handshakes	{ this, "num_received_handshakes" };
	xf::ModuleOut<uint64_t>		num_correct_handshakes	{ this, "num_correct_handshakes" };
	// It's non-nil when responding to a handshake, becomes nil after the handshake is complete.
	xf::ModuleOut<std::string>	handshake_response		{ this, "handshake_response" };

  private:
	static constexpr char kLoggerScope[] = "mod::SlaveTransceiver";

    /**
	 * An two-way communication session.
     */
	class Session:
		public Transceiver::Session,
		public neutrino::Noncopyable
	{
		static inline size_t _id_generator = 0;

	  public:
		// Ctor
		explicit
		Session (Blob const& handshake_request,
				 CryptoParams const&,
				 HandshakeSlave::KeyCheckFunctions const);

		[[nodiscard]]
		Secure<Blob> const&
		handshake_response() const noexcept
			{ return _handshake_response; }

		[[nodiscard]]
		Blob
		tx_key_hash() const noexcept
			{ return _transmitter->data_encryption_key_hash(); }

		[[nodiscard]]
		Blob
		rx_key_hash() const noexcept
			{ return _receiver->data_encryption_key_hash(); }

		// Session API
		[[nodiscard]]
		bool
		connected() const override
			{ return true; }

		// Session API
		[[nodiscard]]
		Transmitter const&
		transmitter() const override
			{ return *_transmitter; }

		// Session API
		[[nodiscard]]
		Blob
		encrypt_packet (BlobView const packet)
			{ return _transmitter->encrypt_packet (packet); }

		// Session API
		[[nodiscard]]
		Blob
		decrypt_packet (BlobView const packet, std::optional<Transport::SequenceNumber> const maximum_allowed_sequence_number = std::nullopt)
			{ return _receiver->decrypt_packet (packet, maximum_allowed_sequence_number); }

	  private:
		Secure<Blob>						_handshake_response;
		std::optional<Transmitter>			_transmitter;
		std::optional<Receiver>				_receiver;
		HandshakeSlave::KeyCheckFunctions	_key_check_functions;
	};

  public:
	// Ctor
	explicit
	SlaveTransceiver (xf::ProcessingLoop&,
					  CryptoParams const&,
					  HandshakeSlave::KeyCheckFunctions const,
					  xf::Logger const&,
					  std::string_view const instance = {});

	/**
	 * Disconnect active connection if connected.
	 */
	void
	disconnect() noexcept
		{ _active_session.reset(); }

	[[nodiscard]]
	std::optional<Blob>
	tx_key_hash() const;

	[[nodiscard]]
	std::optional<Blob>
	rx_key_hash() const;

  protected:
	// Module API
	void
	process (Cycle const&);

	// Transceiver API
	[[nodiscard]]
	Session*
	previous_session() override
		{ return nullptr; }

	// Transceiver API
	[[nodiscard]]
	Session const*
	previous_session() const override
		{ return nullptr; }

	// Transceiver API
	[[nodiscard]]
	Session*
	active_session() override
		{ return _active_session.get(); }

	// Transceiver API
	[[nodiscard]]
	Session const*
	active_session() const override
		{ return _active_session.get(); }

	// Transceiver API
	[[nodiscard]]
	Session*
	next_session_candidate() override
		{ return _next_session_candidate.get(); }

	// Transceiver API
	[[nodiscard]]
	Session const*
	next_session_candidate() const override
		{ return _next_session_candidate.get(); }

	// Transceiver API
	void
	shift_sessions() override;

  private:
	CryptoParams						_crypto_params;
	xf::SocketValueChanged<std::string>	_handshake_request_changed { handshake_request };
	HandshakeSlave::KeyCheckFunctions	_key_check_functions;
	// Active session means connected and confirmed correct encryption+authentication:
	std::unique_ptr<Session>			_active_session;
	std::unique_ptr<Session>			_next_session_candidate;
};

} // namespace xf::crypto::xle

#endif

