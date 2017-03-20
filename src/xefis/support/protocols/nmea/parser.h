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

#ifndef XEFIS__SUPPORT__PROTOCOLS__NMEA__PARSER_H__INCLUDED
#define XEFIS__SUPPORT__PROTOCOLS__NMEA__PARSER_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/support/protocols/nmea/nmea.h>

// Local:
#include "gps.h"
#include "mtk.h"


namespace xf::nmea {

/**
 * Parser for NMEA protocol for GPS devices.
 */
class Parser: private Noncopyable
{
  public:
	/**
	 * Listener class which methods will be called
	 * back when parsing NMEA sentences.
	 */
	class Listener
	{
	  public:
		/**
		 * GPGGA sentence was parsed.
		 */
		virtual void
		process_nmea_sentence (GPGGA const&) = 0;

		/**
		 * GPGSA sentence was parsed.
		 */
		virtual void
		process_nmea_sentence (GPGSA const&) = 0;

		/**
		 * GPRMC sentence was parsed.
		 */
		virtual void
		process_nmea_sentence (GPRMC const&) = 0;

		/**
		 * PMTK ACK sentence was parsed.
		 */
		virtual void
		process_nmea_sentence (PMTKACK const&) = 0;
	};

  public:
	/**
	 * Ctor
	 * \param	Listener
	 *			Callbacks object.
	 */
	explicit
	Parser (Listener*);

	/**
	 * Feed the parser with data received from GPS module.
	 * Don't parse it and don't call any listeners. For that,
	 * use the process_one() method.
	 */
	void
	feed (Blob const& gps_data);

	/**
	 * Parse single sentence from the input buffer.
	 * \throws	Any exception thrown by sentence constructor.
	 * \return	True if there was any sentence processed.
	 */
	bool
	process_one();

  public:
	/**
	 * Verify that NMEA sentence is valid and has proper
	 * checksum.
	 * \throws	NMEA exceptions: InvalidType, InvalidChecksum, InvalidMessage.
	 */
	void
	verify_sentence (std::string const& sentence);

  private:
	std::string		_input_buffer;
	bool			_synchronized	= false;
	Listener*		_listener;
};

} // namespace xf::nmea

#endif

