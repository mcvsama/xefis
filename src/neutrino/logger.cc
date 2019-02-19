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

// Boost:
#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>

// Neutrino:
#include <neutrino/time_helper.h>
#include <neutrino/variant.h>

// Local:
#include "logger.h"


namespace neutrino {

void
LoggerOutput::log (LogBlock const& block)
{
	std::lock_guard lock (_stream_mutex);

	if (_add_timestamps)
		_stream << '[' << LoggerOutput::kTimestampColor << boost::format ("%08.4lfs") % block.timestamp().in<si::Second>() << LoggerOutput::kResetColor << ']';

	_stream << block.string();
}


Logger
Logger::with_scope (std::string_view const& additional_scope) const
{
	Logger new_one (*this);
	new_one.add_scope (additional_scope);
	return new_one;
}


void
Logger::compute_scope()
{
	using namespace std::literals;

	_computed_scope = "["s + LoggerOutput::kScopeColor + boost::algorithm::join (scopes(), LoggerOutput::kResetColor + "]["s + LoggerOutput::kScopeColor) + LoggerOutput::kResetColor + "]";
}


std::string
Logger::prepare_line() const
{
	std::ostringstream prefix;

	if (_logger_tag_provider)
		if (auto tag = _logger_tag_provider->logger_tag())
			prefix << '[' << LoggerOutput::kCycleColor << *tag << LoggerOutput::kResetColor << ']';

	prefix << _computed_scope << " ";
	return prefix.str();
}

} // namespace neutrino

