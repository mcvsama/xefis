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

// Local:
#include "packet_reader.h"


namespace Xefis {

PacketReader::PacketReader (std::string const& magic, ParseCallback parse):
	_magic (magic),
	_parse (parse)
{
	if (_magic.empty())
		throw Xefis::Exception ("magic value must not be empty");
}


void
PacketReader::set_minimum_packet_size (std::size_t bytes) noexcept
{
	_minimum_packet_size = bytes;
}


void
PacketReader::set_buffer_capacity (std::size_t bytes) noexcept
{
	_capacity = bytes;
	_buffer.reserve (_capacity);
}


void
PacketReader::feed (std::string const& data)
{
	if (_capacity > 0 && _buffer.size() + data.size() > _capacity)
	{
		// Trim input buffer:
		if (data.size() > _capacity)
			_buffer = data.substr (data.size() - _capacity);
		else
		{
			_buffer.erase (0, _buffer.size() + data.size() - _capacity);
			_buffer += data;
		}
	}
	else
		_buffer += data;

	for (;;)
	{
		std::string::size_type p = _buffer.find (_magic);
		// If magic found:
		if (p != std::string::npos)
		{
			// Everything until packet magic is considered gibberish:
			_buffer.erase (0, p);
			// If enough data to parse:
			if (_buffer.size() >= _minimum_packet_size)
			{
				std::size_t parsed_bytes = _parse();
				if (parsed_bytes > 0)
				{
					_buffer.erase (0, parsed_bytes);
					// If buffer is still non-empty, try parsing again:
					if (!_buffer.empty())
						continue;
				}
			}
		}
		break;
	}
}

} // namespace Xefis

