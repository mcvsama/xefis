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

// Local:
#include "udp_transceiver_with_codec.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


UDPTransceiverWithCodec::UDPTransceiverWithCodec (xf::ProcessingLoop& loop,
												  UDPTransceiver::Parameters const& udp_transceiver_parameters,
												  std::unique_ptr<LinkProtocol> encoder_protocol,
												  LinkEncoder::Parameters const& encoder_parameters,
												  std::unique_ptr<LinkProtocol> decoder_protocol,
												  LinkDecoder::Parameters const& decoder_parameters,
												  nu::Logger const& logger,
												  std::string_view const instance):
	_udp_transceiver (loop, udp_transceiver_parameters, logger, instance),
	_encoder (loop, std::move (encoder_protocol), encoder_parameters, logger, "→ " + instance),
	_decoder (loop, std::move (decoder_protocol), decoder_parameters, logger, "← " + instance)
{
	_udp_transceiver.send << _encoder.encoded_output;
	_decoder.encoded_input << _udp_transceiver.receive;
}
