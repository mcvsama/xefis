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
#include <xefis/config/all.h>
#include <xefis/utility/mutex.h>
#include <xefis/utility/resource.h>
#include <xefis/utility/hextable.h>

// Local:
#include "parser.h"
#include "exceptions.h"


namespace xf::nmea {

static HexTable	$hextable;


Parser::Parser (Listener* listener):
	_listener (listener)
{ }


void
Parser::feed (Blob const& data)
{
	_input_buffer.insert (_input_buffer.end(), data.begin(), data.end());
}


bool
Parser::process_one()
{
	// Skip cut-in-half messages, wait for '$' if not yet synchronized:
	if (!_synchronized)
	{
		auto pos = _input_buffer.find ("$");

		if (pos == std::string::npos)
			return false;
		else
		{
			_input_buffer.erase (0, pos);
			_synchronized = true;
		}
	}

	// Process all sentences terminated with "\r\n".
	constexpr std::string::size_type start = 0;
	std::string::size_type crlf = 0;
	std::string::size_type parsed = 0;

	// Make sure to remove parsed data from the input buffer:
	Resource remove_parsed_properties ([&] {
		_input_buffer.erase (0, parsed);
	});

	crlf = _input_buffer.find ("\r\n", start);
	if (crlf == std::string::npos)
		return false;
	parsed = crlf + 2;

	std::string sentence_str = _input_buffer.substr (start, crlf - start);
	verify_sentence (sentence_str);

	// Extract sentence contents (strip '$' and checksum).
	std::string sentence_meat;
	// Check if we have checksum (this assumes that normal messages never contain
	// an asterisk and it's a reserved character):
	if (sentence_str[sentence_str.size() - 3] == '*')
		sentence_meat = sentence_str.substr (1, sentence_str.size() - 4);
	else
		sentence_meat = sentence_str.substr (1);

	try {
		switch (get_sentence_type (sentence_str))
		{
			case SentenceType::GPGGA:
				_listener->process_nmea_sentence (GPGGA (sentence_meat));
				break;

			case SentenceType::GPGSA:
				_listener->process_nmea_sentence (GPGSA (sentence_meat));
				break;

			case SentenceType::GPRMC:
				_listener->process_nmea_sentence (GPRMC (sentence_meat));
				break;

			case SentenceType::PMTKACK:
				_listener->process_nmea_sentence (PMTKACK (sentence_meat));
				break;
		}
	}
	catch (UnsupportedSentenceType const&)
	{
		// Ignore unsupported sentences.
	}

	return true;
}


void
Parser::verify_sentence (std::string const& sentence)
{
	// Verify checksum:
	if (sentence.size() < 5)
		throw nmea::InvalidSentence ("NMEA sentence too short");

	// Prologue:
	if (sentence[0] != '$')
		throw nmea::InvalidSentence ("NMEA sentence should start with '$'");

	// Parse checksum if present:
	if (sentence[sentence.size() - 3] == '*')
	{
		char c1 = sentence[sentence.size() - 2];
		char c2 = sentence[sentence.size() - 1];
		if (!std::isxdigit (c1) || !std::isxdigit (c2))
			throw nmea::InvalidSentence ("checksum characters are not valid hex digits");
		uint8_t parsed_checksum = $hextable[c1] * 16 + $hextable[c2];

		// Verify checksum:
		uint8_t expected_checksum = 0;
		for (auto c = sentence.begin() + 1; c != sentence.end() - 3; ++c)
			expected_checksum ^= *c;

		if (expected_checksum != parsed_checksum)
			throw nmea::InvalidChecksum (expected_checksum, parsed_checksum);
	}
}

} // namespace xf::nmea

