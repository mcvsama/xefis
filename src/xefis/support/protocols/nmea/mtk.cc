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

// Local:
#include "mtk.h"
#include "exceptions.h"
#include "nmea.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>
#include <string>


namespace xf::nmea {

PMTKACK::PMTKACK (std::string const& sentence):
	Sentence (sentence)
{
	if (!read_next() || val() != "PMTK001")
		throw InvalidType ("PMTK001", val());

	// Command info:
	if (!read_next())
		return;

	this->command = val();

	if (!read_next())
		return;

	// Result:
	if (val() == "0")
		this->result = MTKResult::InvalidCommand;
	else if (val() == "1")
		this->result = MTKResult::UnsupportedCommand;
	else if (val() == "2")
		this->result = MTKResult::Failure;
	else if (val() == "3")
		this->result = MTKResult::Success;
}


std::string
describe_mtk_command_by_id (std::string command)
{
	static std::map<std::string, std::string> const hints {
		{ "101", "hot start" },
		{ "102", "warm start" },
		{ "103", "cold start" },
		{ "104", "full cold start" },
		{ "220", "set NMEA update rate" },
		{ "251", "set baud rate" },
		{ "286", "enable/disable AIC mode" },
		{ "300", "set fixing rate" },
		{ "301", "set DGPS mode" },
		{ "313", "enable/disable SBAS" },
		{ "314", "set NMEA frequencies" },
		{ "319", "set SBAS mode" },
		{ "513", "enable/disable SBAS" },
	};

	if (auto h = hints.find (command); h != hints.end())
		return h->second;
	else
		return {};
}


std::string
make_mtk_sentence (std::string const& data)
{
	return "$" + data + "*" + make_checksum (data) + "\r\n";
}

} // namespace xf::nmea

