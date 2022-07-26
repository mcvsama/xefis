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
#include "packet_reader.h"

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <algorithm>


namespace xf {

PacketReader::PacketReader (Blob const& magic, ParseCallback parse):
	_magic (magic),
	_parse (parse)
{
	if (_magic.empty())
		throw Exception ("magic value must not be empty");
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
PacketReader::feed (Blob const& data)
{
	using neutrino::to_signed;

	if (_capacity > 0 && _buffer.size() + data.size() > _capacity)
	{
		// Trim input buffer:
		if (data.size() > _capacity)
			_buffer = Blob (data.begin() + to_signed (data.size()) - to_signed (_capacity), data.end());
		else
		{
			_buffer.erase (_buffer.begin(), _buffer.begin() + to_signed (_buffer.size()) + to_signed (data.size()) - to_signed (_capacity));
			_buffer.insert (_buffer.end(), data.begin(), data.end());
		}
	}
	else
		_buffer.insert (_buffer.end(), data.begin(), data.end());

	for (;;)
	{
		// Find magic string in buffer:
		Blob::iterator p = std::search (_buffer.begin(), _buffer.end(),
										_magic.begin(), _magic.end());
		// If magic found:
		if (p != _buffer.end())
		{
			// Everything until packet magic is considered gibberish:
			_buffer.erase (_buffer.begin(), p);
			// If enough data to parse:
			if (_buffer.size() >= _minimum_packet_size)
			{
				std::size_t parsed_bytes = _parse();
				if (parsed_bytes > 0)
				{
					_buffer.erase (_buffer.begin(), _buffer.begin() + to_signed (parsed_bytes));
					// If buffer is still non-empty, try parsing again:
					if (!_buffer.empty())
						continue;
				}
			}
		}
		break;
	}
}

} // namespace xf

