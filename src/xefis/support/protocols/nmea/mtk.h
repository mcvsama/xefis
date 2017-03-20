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

#ifndef XEFIS__SUPPORT__PROTOCOLS__NMEA__MTK_H__INCLUDED
#define XEFIS__SUPPORT__PROTOCOLS__NMEA__MTK_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "nmea.h"


namespace xf::nmea {

enum class MTKResult
{
	InvalidCommand		= 0,
	UnsupportedCommand	= 1,
	Failure				= 2,
	Success				= 3,
};


/**
 * PMTK ACK message.
 */
class PMTKACK: public Sentence
{
  public:
	/**
	 * Ctor
	 * Parse PMTK ACK message.
	 * \throws	InvalidType if message header isn't 'PMTK001'.
	 */
	explicit
	PMTKACK (std::string const&);

  public:
	// Command to which this ACK responds to:
	Optional<std::string>	command;

	// Result:
	Optional<MTKResult>		result;
};


/**
 * Return string describing MTK command.
 * Command must be of form "PMTKnnn".
 */
extern std::string
describe_mtk_command_by_id (std::string command);


/**
 * Create MTK message. Data must include message name: PMTKnnn,
 * where nnn is message ID.
 */
extern std::string
make_mtk_sentence (std::string const& data);

} // namespace xf::nmea

#endif

