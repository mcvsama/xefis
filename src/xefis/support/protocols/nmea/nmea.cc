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

// Lib:
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/mutex.h>
#include <xefis/utility/numeric.h>

// Local:
#include "nmea.h"
#include "exceptions.h"


namespace xf::nmea {

Sentence::Sentence (std::string const& sentence)
{
	// Copy sentence last, when passed all tests:
	_sentence = sentence;
}


bool
Sentence::read_next()
{
	if (_pos == std::string::npos)
	{
		_val.clear();
		return false;
	}

	auto start_pos = _pos;

	std::string::size_type comma = _sentence.find (',', start_pos);

	if (comma == std::string::npos)
	{
		_val.resize (_sentence.size() - start_pos);
		std::copy (_sentence.begin() + start_pos, _sentence.end(), _val.begin());
		_pos = std::string::npos;
	}
	else
	{
		_val.resize (comma - start_pos);
		std::copy (_sentence.begin() + start_pos, _sentence.begin() + comma, _val.begin());
		_pos = comma + 1;
	}

	return true;
}


bool
Sentence::read_latitude (std::optional<Angle>& latitude)
{
	if (!read_next())
		return false;

	if (val().size() >= 3)
	{
		auto lat = 1_deg * (digit_from_ascii (val()[0]) * 10 +
							digit_from_ascii (val()[1]));
		try {
			latitude = lat + 1_deg * boost::lexical_cast<double> (val().substr (2)) / 60.0;
		}
		catch (boost::bad_lexical_cast&)
		{ }
	}

	// North/South:
	if (!read_next())
	{
		latitude.reset();
		return false;
	}

	if (val() == "S")
		latitude = -1 * *latitude;
	else if (val() != "N")
		latitude.reset();

	return true;
}


bool
Sentence::read_longitude (std::optional<Angle>& longitude)
{
	if (!read_next())
		return false;

	if (val().size() >= 4)
	{
		auto lon = 1_deg * (digit_from_ascii (val()[0]) * 100 +
							digit_from_ascii (val()[1]) * 10 +
							digit_from_ascii (val()[2]));
		try {
			longitude = lon + 1_deg * boost::lexical_cast<double> (val().substr (3)) / 60.0;
		}
		catch (boost::bad_lexical_cast&)
		{ }
	}

	// East/West:
	if (!read_next())
	{
		longitude.reset();
		return false;
	}

	if (val() == "W")
		longitude = -1 * *longitude;
	else if (val() != "E")
		longitude.reset();

	return true;
}


std::string
make_checksum (std::string const& data)
{
	uint8_t sum = 0;
	for (auto c: data)
		sum ^= c;
	return (boost::format ("%02X") % static_cast<int> (sum)).str();
}


SentenceType
get_sentence_type (std::string const& sentence)
{
	if (sentence.compare (0, 7, "$GPGGA,") == 0)
		return SentenceType::GPGGA;
	else if (sentence.compare (0, 7, "$GPGSA,") == 0)
		return SentenceType::GPGSA;
	else if (sentence.compare (0, 7, "$GPRMC,") == 0)
		return SentenceType::GPRMC;
	else if (sentence.compare (0, 9, "$PMTK001,") == 0)
		return SentenceType::PMTKACK;
	else
		throw UnsupportedSentenceType (sentence);
}

} // namespace xf::nmea

