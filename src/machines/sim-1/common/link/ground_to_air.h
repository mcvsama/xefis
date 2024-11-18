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

#ifndef XEFIS__MACHINES__SIM_1__COMMON__LINK__GROUND_TO_AIR_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__COMMON__LINK__GROUND_TO_AIR_H__INCLUDED

// Xefis:
#include <xefis/core/module.h>
#include <xefis/modules/comm/link/link_protocol.h>
#include <xefis/modules/comm/xle_transceiver.h>


namespace sim1 {

template<template<class> class SocketType>
	class GroundToAirData: public xf::Module
	{
	  public:
		SocketType<std::string>			encryption_handshake_request	{ this, "encryption/handshake_request" };
		// Joystick:
		SocketType<double>				joystick_pitch					{ this, "joystick/pitch" };
		SocketType<double>				joystick_roll					{ this, "joystick/roll" };
		SocketType<double>				joystick_yaw					{ this, "joystick/yaw" };
		// Trims:
		SocketType<double>				trim_pitch						{ this, "trim/pitch" };
		SocketType<double>				trim_roll						{ this, "trim/roll" };
		SocketType<double>				trim_yaw						{ this, "trim/yaw" };
		// Throttle:
		SocketType<double>				throttle_left					{ this, "throttle/left" };
		SocketType<double>				throttle_right					{ this, "throttle/right" };

	  public:
		// Ctor
		using Module::Module;
	};


class GroundToAirProtocol: public LinkProtocol
{
  public:
	template<class Data>
		explicit
		GroundToAirProtocol (Data& data, xf::crypto::xle::Transceiver& transceiver):
			LinkProtocol ({
				// XLE handshake envelope:
				envelope ({
					.unique_prefix	= { 0xf3, 0x3f },
					// Only send this envelope when handshake request is ready:
					.send_predicate	= [&data] { return data.encryption_handshake_request.valid(); },
					.packets		= {
						// Always good to have at least basic checksum:
						signature ({
							.nonce_bytes		= 0,
							.signature_bytes	= 4,
							.key				= neutrino::to_blob ("ground-to-air-handshake"),
							.packets			= {
								socket<256> (data.encryption_handshake_request, { .retained = false }),
							},
						}),
					},
				}),
				envelope ({
					.unique_prefix	= { 0xfe, 0x5a },
					.transceiver	= &transceiver,
					.packets		= {
						socket<4> (data.joystick_pitch),
						socket<4> (data.joystick_roll),
						socket<4> (data.joystick_yaw),
						socket<4> (data.trim_pitch),
						socket<4> (data.trim_roll),
						socket<4> (data.trim_yaw),
						socket<4> (data.throttle_left),
						socket<4> (data.throttle_right),
					},
				}),
			})
		{ }
};

} // namespace sim1

#endif

