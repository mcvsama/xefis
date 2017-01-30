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

#ifndef XEFIS__SUPPORT__PROTOCOLS__NMEA__EXCEPTIONS_H__INCLUDED
#define XEFIS__SUPPORT__PROTOCOLS__NMEA__EXCEPTIONS_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Lib:
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>


namespace xf::nmea {

/**
 * Thrown when trying to construct a NMEA sentence object
 * from non-compatible NMEA string sentence.
 */
class InvalidType: public Exception
{
  public:
	/**
	 * \param	expected_header
	 *			NMEA header that was expected.
	 * \param	actual_header
	 *			Actual header from the string.
	 */
	InvalidType (std::string const& expected_header, std::string const& actual_header):
		Exception ("unexpected NMEA sentence header '" + actual_header + "', expected '" + expected_header + "'")
	{ }
};


/**
 * Thrown when sentence checksum doesn't match sentence contents.
 */
class InvalidChecksum: public Exception
{
  public:
	InvalidChecksum (uint8_t expected_checksum, uint8_t actual_checksum):
		// static_cast because of boost bug when formatting uint8_ts:
		Exception ((boost::format ("invalid NMEA sentence checksum '%02x', should be '%02x'") % static_cast<int> (actual_checksum) % static_cast<int> (expected_checksum)).str())
	{ }
};


/**
 * Thrown on general sentence parse error.
 */
class InvalidSentence: public Exception
{
  public:
	using Exception::Exception;
};

} // namespace xf::nmea

#endif

