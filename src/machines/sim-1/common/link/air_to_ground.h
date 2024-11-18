/* vim:ts=4
 *
 * Copyleft 2008…2023  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIM_1__COMMON__LINK__AIR_TO_GROUND_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__COMMON__LINK__AIR_TO_GROUND_H__INCLUDED

// Xefis:
#include <xefis/core/module.h>
#include <xefis/modules/comm/link/link_protocol.h>
#include <xefis/modules/comm/xle_transceiver.h>


namespace sim1 {

template<template<class> class SocketType>
	class AirToGroundData: public xf::Module
	{
	  public:
		SocketType<std::string>			encryption_handshake_response	{ this, "encryption/handshake_response" };
		SocketType<si::Pressure>		static_pressure					{ this, "sensors/pressure/static" };
		SocketType<si::Pressure>		total_pressure					{ this, "sensors/pressure/total" };

	  public:
		// Ctor
		using Module::Module;
	};


class AirToGroundProtocol: public LinkProtocol
{
  public:
	template<class Data>
		explicit
		AirToGroundProtocol (Data& data, xf::crypto::xle::Transceiver& transceiver):
			LinkProtocol ({
				// XLE handshake envelope:
				envelope ({
					.unique_prefix	= { 0xaf, 0xfa },
					// Only send this envelope when handshake response is ready:
					.send_predicate	= [&data] { return data.encryption_handshake_response.valid(); },
					.packets		= {
						// Always good to have at least basic checksum:
						signature ({
							.nonce_bytes		= 0,
							.signature_bytes	= 4,
							.key				= neutrino::to_blob ("air-to-ground-handshake"),
							.packets			= {
								socket<256> (data.encryption_handshake_response, { .retained = false }),
							},
						}),
					},
				}),
				// Normal data envelope:
				envelope ({
					.unique_prefix	= { 0xf6, 0x6f },
					.transceiver	= &transceiver,
					.packets		= {
						socket<4> (data.static_pressure),
						socket<4> (data.total_pressure),
					},
				}),
			})
		{ }
};

} // namespace sim1

#endif

