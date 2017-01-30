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

// Standard:
#include <cstddef>
#include <string>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/mutex.h>

// Local:
#include "mtk.h"
#include "exceptions.h"
#include "nmea.h"


namespace xf::nmea {

static Mutex $describe_mtk_command_entry_mutex;


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
	// Must acquire lock before static variables initialization:
	auto lock = $describe_mtk_command_entry_mutex.acquire_lock();

	static std::map<std::string, std::string> hints;

	if (hints.empty())
	{
		hints["101"] = "hot start";
		hints["102"] = "warm start";
		hints["103"] = "cold start";
		hints["104"] = "full cold start";
		hints["220"] = "set NMEA update rate";
		hints["251"] = "set baud rate";
		hints["286"] = "enable/disable AIC mode";
		hints["300"] = "set fixing rate";
		hints["301"] = "set DGPS mode";
		hints["313"] = "enable/disable SBAS";
		hints["314"] = "set NMEA frequencies";
		hints["319"] = "set SBAS mode";
		hints["513"] = "enable/disable SBAS";
	}

	auto h = hints.find (command);
	if (h != hints.end())
		return h->second;
	return std::string();
}


std::string
make_mtk_sentence (std::string const& data)
{
	return "$" + data + "*" + make_checksum (data) + "\r\n";
}

} // namespace xf::nmea

