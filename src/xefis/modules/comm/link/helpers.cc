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

// Local:
#include "helpers.h"


std::unique_ptr<LinkProtocol>
make_link_protocol_from_inputs (xf::Module& module, std::string_view const envelope_name, nu::Blob const& envelope_unique_prefix)
{
	return make_link_protocol_from_inputs ({ &module }, envelope_name, envelope_unique_prefix);
}


std::unique_ptr<LinkProtocol>
make_link_protocol_from_inputs (std::vector<xf::Module*> modules, std::string_view const envelope_name, nu::Blob const& envelope_unique_prefix)
{
	auto collected_packets = LinkProtocol::PacketList();

	for (auto* module: modules)
		collected_packets.append_range (make_packets_from_inputs (*module));

	return std::make_unique<LinkProtocol> (LinkProtocol::EnvelopeList {
		LinkProtocol::envelope ({
			.name = envelope_name,
			.unique_prefix = envelope_unique_prefix,
			.packets = collected_packets,
		}),
	});
}


std::unique_ptr<LinkProtocol>
make_link_protocol_from_outputs (xf::Module& module, std::string_view const envelope_name, nu::Blob const& envelope_unique_prefix)
{
	return std::make_unique<LinkProtocol> (LinkProtocol::EnvelopeList {
		LinkProtocol::envelope ({
			.name = envelope_name,
			.unique_prefix = envelope_unique_prefix,
			.packets = make_packets_from_outputs (module),
		}),
	});
}


std::unique_ptr<LinkProtocol>
make_link_protocol_from_outputs (std::vector<xf::Module*> modules, std::string_view const envelope_name, nu::Blob const& envelope_unique_prefix)
{
	auto collected_packets = LinkProtocol::PacketList();

	for (auto* module: modules)
		collected_packets.append_range (make_packets_from_outputs (*module));

	return std::make_unique<LinkProtocol> (LinkProtocol::EnvelopeList {
		LinkProtocol::envelope ({
			.name = envelope_name,
			.unique_prefix = envelope_unique_prefix,
			.packets = collected_packets,
		}),
	});
}
