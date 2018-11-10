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

// Xefis:
#include <xefis/core/processing_loop.h>
#include <xefis/utility/variant.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "logger.h"


namespace xf {

void
LoggerOutput::log (LogBlock const& block)
{
	std::lock_guard lock (_stream_mutex);

	if (_add_timestamps)
		_stream << '[' << LoggerOutput::kTimestampColor << boost::format ("%08.4lfs") % block.timestamp().in<Second>() << LoggerOutput::kResetColor << ']';

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

	if (_processing_loop)
	{
		if (auto cycle = _processing_loop->current_cycle())
			prefix << '[' << LoggerOutput::kCycleColor << boost::format ("cycle=%08d") % cycle->number() << LoggerOutput::kResetColor << ']';
		else
			prefix << '[' << LoggerOutput::kCycleColor << "cycle=--------" << LoggerOutput::kResetColor << ']';
	}

	prefix << _computed_scope << " ";
	return prefix.str();
}

} // namespace xf

