/* vim:ts=4
 *
 * Copyleft 2023  Michał Gawron
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
#include "link_encoder.h"

// Neutrino:
#include <neutrino/stdexcept.h>


using namespace nu::si::literals;


LinkEncoder::LinkEncoder (xf::ProcessingLoop& loop,
						  std::unique_ptr<LinkProtocol> protocol,
						  Parameters const& params,
						  nu::Logger const& logger,
						  std::string_view const instance):
	Module (loop, instance),
	_logger (logger.with_context (std::string (kLoggerScope) + "#" + instance)),
	_protocol (std::move (protocol)),
	_send_period (1 / params.send_frequency)
{
	if (!_protocol)
		throw nu::InvalidArgument ("LinkEncoder: 'protocol' must not be nullptr");

	_output_blob.reserve (2 * _protocol->size());
}


void
LinkEncoder::process (xf::Cycle const& cycle)
{
	if (cycle.update_time() - _previous_update_time > _send_period)
	{
		send_output (cycle);
		_previous_update_time = cycle.update_time();
	}
}


void
LinkEncoder::send_output (xf::Cycle const& cycle)
{
	_output_blob.clear();
	_protocol->produce_append (_output_blob, cycle.logger() + _logger);
	this->encoded_output = std::string (_output_blob.begin(), _output_blob.end());
}
