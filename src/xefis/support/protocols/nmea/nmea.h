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

#ifndef XEFIS__SUPPORT__PROTOCOLS__NMEA__NMEA_H__INCLUDED
#define XEFIS__SUPPORT__PROTOCOLS__NMEA__NMEA_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Xefis:
#include <xefis/config/all.h>


namespace xf::nmea {

/**
 * Thrown when NMEA or PMTK sentence type can't be resolved.
 */
class UnsupportedSentenceType: public Exception
{
  public:
	explicit
	UnsupportedSentenceType (std::string const& sentence);
};


/**
 * Sentence type.
 */
enum class SentenceType
{
	GPGGA,		// GPS fix information
	GPGSA,		// GPS overall satellite data
	GPRMC,		// GPS recommended minimum data
	PMTKACK,	// MTK ACK
};


/**
 * Common base for all NMEA sentences.
 */
class Sentence
{
  protected:
	/**
	 * Ctor
	 * \param	sentence
	 *			String between the '$' and '*'.
	 */
	explicit
	Sentence (std::string const&);

  public:
	/**
	 * Return sentence contents (without prolog and checksum).
	 */
	std::string const&
	contents() const noexcept;

  protected:
	/**
	 * Get next substring up to next comma or end of string.
	 * The string is avaiable through val() method.
	 *
	 * \return	true if OK, false if end of string reached in the
	 *			previous call to this method.
	 */
	bool
	read_next();

	/**
	 * \return	substring extraced with read_to_comma().
	 */
	std::string const&
	val() const noexcept;

	/**
	 * Read latitude (using standard read_next()).
	 *
	 * \return	false if read_next() returned false internally,
	 *			so it's time to finish.
	 */
	bool
	read_latitude (Optional<Angle>& out_latitude);

	/**
	 * Read longitude (using standard read_next()).
	 *
	 * \return	false if read_next() returned false internally,
	 *			so it's time to finish.
	 */
	bool
	read_longitude (Optional<Angle>& out_longitude);

  private:
	std::string				_sentence;
	std::string				_val;
	std::string::size_type	_pos	= 0;
};


inline std::string const&
Sentence::contents() const noexcept
{
	return _sentence;
}


inline std::string const&
Sentence::val() const noexcept
{
	return _val;
}


/**
 * Make NMEA checksum from the input string.
 * \param	data
 *			String between '$' and '*' (exclusive).
 * \return	two-character checksum (do not include '*').
 */
extern std::string
make_checksum (std::string const& data);


/**
 * Parse header of the sentence and return sentence type.
 * String may include the first '$' character of NMEA sentence.
 */
extern SentenceType
get_sentence_type (std::string const& sentence);

} // namespace xf::nmea

#endif

