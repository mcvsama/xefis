/* vim:ts=4
 *
 * Copyleft 2012…2023  Michał Gawron
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
#include "output_link.h"


using namespace neutrino::si::literals;


OutputLink::OutputLink (xf::ProcessingLoop& loop, std::unique_ptr<LinkProtocol> protocol, si::Frequency const send_frequency, xf::Logger const& logger, std::string_view const& instance):
	Module (loop, instance),
	_logger (logger.with_context (std::string (kLoggerScope) + "#" + instance)),
	_protocol (std::move (protocol)),
	_send_period (1 / send_frequency)
{
	_output_blob.reserve (2 * _protocol->size());
}


void
OutputLink::process (xf::Cycle const& cycle)
{
	if (cycle.update_time() - _previous_update_time > _send_period)
	{
		send_output();
		_previous_update_time = cycle.update_time();
	}
}


void
OutputLink::send_output()
{
	_output_blob.clear();
	_protocol->produce (_output_blob, _logger);
	this->link_output = std::string (_output_blob.begin(), _output_blob.end());
}

