/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__COMM__LINK__HELPERS_H__INCLUDED
#define XEFIS__MODULES__COMM__LINK__HELPERS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/modules/comm/link/link_protocol.h>

// Neutrino:
#include <neutrino/blob.h>
#include <neutrino/concepts.h>

// Standard:
#include <cstddef>
#include <memory>
#include <ranges>
#include <string_view>


namespace nu = neutrino;


[[nodiscard]]
std::unique_ptr<LinkProtocol>
make_link_protocol_from_inputs (xf::Module&, std::string_view envelope_name, nu::Blob const& envelope_unique_prefix);


[[nodiscard]]
std::unique_ptr<LinkProtocol>
make_link_protocol_from_inputs (std::vector<xf::Module*>, std::string_view envelope_name, nu::Blob const& envelope_unique_prefix);


[[nodiscard]]
std::unique_ptr<LinkProtocol>
make_link_protocol_from_outputs (xf::Module&, std::string_view envelope_name, nu::Blob const& envelope_unique_prefix);


[[nodiscard]]
std::unique_ptr<LinkProtocol>
make_link_protocol_from_outputs (std::vector<xf::Module*>, std::string_view envelope_name, nu::Blob const& envelope_unique_prefix);


/**
 * Make LinkProtocol::PacketList from provided subrange of sockets.
 */
template<std::ranges::range SocketsRange>
	[[nodiscard]]
	inline LinkProtocol::PacketList
	make_packets_from (SocketsRange sockets)
		requires std::is_base_of_v<xf::BasicSocket, std::remove_pointer_t<std::ranges::range_value_t<SocketsRange>>> ||
				 std::is_base_of_v<xf::BasicAssignableSocket, std::remove_pointer_t<std::ranges::range_value_t<SocketsRange>>>
	{
		auto list = LinkProtocol::PacketList();

		for (auto& socket: sockets)
			list.push_back (LinkProtocol::local_socket (*socket));

		return list;
	}


/**
 * Make LinkProtocol::PacketList from all input-type sockets registered in provided module.
 * The result can be then used as .packets field of LinkProtocol::Envelope.
 * The order of sockets is the same as order of registration of sockets in the module.
 */
[[nodiscard]]
inline LinkProtocol::PacketList
make_packets_from_inputs (xf::Module& module)
{
	return make_packets_from (xf::Module::ModuleSocketAPI (module).input_sockets());
}


/**
 * Like make_packets_from_inputs(), but uses output-type sockets.
 */
[[nodiscard]]
inline LinkProtocol::PacketList
make_packets_from_outputs (xf::Module& module)
{
	return make_packets_from (xf::Module::ModuleSocketAPI (module).output_sockets());
}

#endif
