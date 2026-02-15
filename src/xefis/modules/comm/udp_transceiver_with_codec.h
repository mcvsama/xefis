/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__COMM__UDP_WITH_CODEC_H__INCLUDED
#define XEFIS__MODULES__COMM__UDP_WITH_CODEC_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/modules/comm/link/link_encoder.h>
#include <xefis/modules/comm/link/link_decoder.h>
#include <xefis/modules/comm/udp_transceiver.h>

// Neutrino:
#include <neutrino/logger.h>

// Qt:
#include <QtNetwork/QUdpSocket>

// Standard:
#include <cstddef>


namespace nu = neutrino;


class UDPTransceiverWithCodec
{
  private:
	static constexpr char kLoggerScope[] = "mod::UDPTransceiverWithCodec";

  public:
	// Ctor
	explicit
	UDPTransceiverWithCodec (xf::ProcessingLoop&,
							 UDPTransceiver::Parameters const&,
							 std::unique_ptr<LinkProtocol> encoder_protocol,
							 LinkEncoder::Parameters const&,
							 std::unique_ptr<LinkProtocol> decoder_protocol,
							 LinkDecoder::Parameters const&,
							 nu::Logger const&,
							 std::string_view const instance = {});

	UDPTransceiver&
	udp_transceiver() noexcept
		{ return _udp_transceiver; }

	LinkEncoder&
	encoder() noexcept
		{ return _encoder; }

	LinkDecoder&
	decoder() noexcept
		{ return _decoder; }

  private:
	UDPTransceiver	_udp_transceiver;
	LinkEncoder		_encoder;
	LinkDecoder		_decoder;
};

#endif
