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

// Xefis:
#include <xefis/core/processing_loop.h>
#include <xefis/utility/variant.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "logger.h"


namespace xf {

Logger::Logger (std::ostream& stream):
	_output (&stream)
{ }


Logger::Logger (Parent const& parent):
	_output (&*parent)
{ }


Logger::Logger (std::ostream& stream, ProcessingLoop const& processing_loop):
	_output (&stream),
	_processing_loop (&processing_loop)
{ }


Logger::Logger (Parent const& parent, ProcessingLoop const& processing_loop):
	_output (&*parent),
	_processing_loop (&processing_loop)
{ }


void
Logger::set_prefix (std::string const& prefix)
{
	_prefix = "[" + prefix + "]";
}


void
Logger::set_timestamps_enabled (bool enabled)
{
	_add_timestamps = enabled;
}


std::ostream&
Logger::prepare_line() const
{
	auto& stream_to_use = std::visit (xf::overload {
		[&] (std::ostream* stream) noexcept -> std::ostream& {
			return *stream;
		},
		[&] (Logger const* parent_logger) noexcept -> std::ostream& {
			return parent_logger->prepare_line();
		}
	}, _output);

	prepare_line (stream_to_use);
	return stream_to_use;
}


void
Logger::prepare_line (std::ostream& stream) const
{
	if (_processing_loop)
	{
		if (auto cycle = _processing_loop->current_cycle())
			stream << boost::format ("[%08d] ") % cycle->number();
		else
			stream << "[no cycle] ";
	}

	if (_add_timestamps)
		stream << boost::format ("[%08.4lfs] %s ") % TimeHelper::now().in<Second>() % _prefix;
	else
		stream << _prefix;
}

} // namespace xf

