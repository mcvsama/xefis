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

#ifndef XEFIS__SUPPORT__PROTOCOLS__NMEA__GPS_H__INCLUDED
#define XEFIS__SUPPORT__PROTOCOLS__NMEA__GPS_H__INCLUDED

// Local:
#include "nmea.h"

// Xefis:
#include <xefis/config/all.h>


namespace xf::nmea {

class GPSDate;
class GPSTimeOfDay;


/**
 * Thrown when invalid GPS date/time is used in a function.
 */
class BadDateTime: public nu::Exception
{
  public:
	BadDateTime (GPSDate const&, GPSTimeOfDay const&);
};


/**
 * Fix quality information from a GPS device.
 */
enum class GPSFixQuality
{
	Invalid		= 0,
	GPS			= 1,	// GPS
	DGPS		= 2,	// Differential GPS
	PPS			= 3,	// Precision Positioning Service
	RTK			= 4,	// Real-Time Kinematic
	FloatRTK	= 5,	// Floating-point RTK
	Estimated	= 6,	// Dead reckoning
	Manual		= 7,
	Simulated	= 8,
};


/**
 * GPS receiver status.
 */
enum class GPSReceiverStatus
{
	Active,
	Void,
};


/**
 * 2D or 3D fix selection mode.
 */
enum class GPSFixSelectionMode
{
	Auto,
	Manual,
};


/**
 * GPS fix.
 */
enum class GPSFixMode
{
	None,
	Fix2D,
	Fix3D,
};


/**
 * Basic GPS time used by NMEA sentences, always in UTC.
 */
class GPSTimeOfDay
{
  public:
	/**
	 * Ctor
	 * \param	gps_time
	 *			String taken from NMEA message,
	 *			formatted: HHMMSS.
	 */
	explicit
	GPSTimeOfDay (std::string const& gps_time);

  public:
	uint8_t		hours;
	uint8_t		minutes;
	uint8_t		seconds;
	double		seconds_fraction;
};


/**
 * Basic GPS date, UTC.
 */
class GPSDate
{
  public:
	/**
	 * Ctor
	 * \param	gps_date
	 *			String taken from NMEA message,
	 *			formatted: DDMMYY.
	 */
	explicit
	GPSDate (std::string const& gps_date);

  public:
	uint8_t		day;
	uint8_t		month;
	uint16_t	year;
};


/**
 * Fix information sentence.
 */
class GPGGA: public Sentence
{
  public:
	/**
	 * Ctor
	 * Parse NMEA sentence between '$' and '*'.
	 * \throws	InvalidType if message header isn't 'GPGGA'.
	 */
	explicit
	GPGGA (std::string const&);

  public:
	// UTC time when fix was obtained:
	std::optional<GPSTimeOfDay>		fix_time;

	// Latitude, positive is North:
	std::optional<si::Angle>		latitude;

	// Longitude, positive is East:
	std::optional<si::Angle>		longitude;

	// GPS fix quality information:
	std::optional<GPSFixQuality>	fix_quality;

	// Number of satellites being tracked:
	std::optional<unsigned int>		tracked_satellites;

	// Horizontal dilution of precision:
	std::optional<float>			hdop;

	// Altitude, above mean sea level:
	std::optional<si::Length>		altitude_amsl;

	/**
	 * Height of geoid (mean sea level) above WGS84 ellipsoid
	 * at current position.
	 *
	 * If the height of geoid is missing then the altitude should
	 * be suspect. Some non-standard implementations report altitude
	 * with respect to the ellipsoid rather than geoid altitude.
	 * Some units do not report negative altitudes at all.
	 * This is the only sentence that reports altitude.
	 */
	std::optional<si::Length>		geoid_height;

	// Time since last DGPS update:
	std::optional<si::Time>			dgps_last_update_time;

	// DGPS station ID number:
	std::optional<uint64_t>			dgps_station_id;

  public:
	/**
	 * Return true if fix is reliable, that is it's not simulated.
	 */
	bool
	reliable_fix_quality() const noexcept;
};


/**
 * GPS DOP and active satellites info.
 */
class GPGSA: public Sentence
{
  public:
	/**
	 * Ctor
	 * Parse NMEA sentence between '$' and '*'.
	 * \throws	InvalidType if message header isn't 'GPGSA'.
	 */
	explicit
	GPGSA (std::string const&);

  public:
	// Fix mode:
	std::optional<GPSFixSelectionMode>			fix_selection_mode;

	// Fix mode:
	std::optional<GPSFixMode>					fix_mode;

	// PRNs of satellites used in the solution:
	std::array<std::optional<unsigned int>, 12>	satellites;

	// PDOP (dilution of precision):
	std::optional<float>						pdop;

	// HDOP (horizontal dilution of precision):
	std::optional<float>						hdop;

	// VDOP (vertical dilution of precision):
	std::optional<float>						vdop;
};


/**
 * GPS position, velocity, time info.
 */
class GPRMC: public Sentence
{
  public:
	/**
	 * Ctor
	 * Parse NMEA sentence between '$' and '*'.
	 * \throws	InvalidType if message header isn't 'GPRMC'.
	 */
	explicit
	GPRMC (std::string const&);

  public:
	// UTC time when fix was obtained:
	std::optional<GPSTimeOfDay>			fix_time;

	// GPSReceiverStatus:
	std::optional<GPSReceiverStatus>	receiver_status;

	// Latitude, positive is North:
	std::optional<si::Angle>			latitude;

	// Longitude, positive is East:
	std::optional<si::Angle>			longitude;

	// Ground-speed:
	std::optional<si::Speed>			ground_speed;

	// Track angle, True direction:
	std::optional<si::Angle>			track_true;

	// Date (UTC):
	std::optional<GPSDate>				fix_date;

	// Magnetic variation;
	std::optional<si::Angle>			magnetic_variation;
};


/**
 * Return string name of the fix quality information,
 * returned by the GPS module.
 */
extern std::string
to_string (GPSFixQuality);


/**
 * Convert GPS date and time to Unix time.
 * Throw BadDateTime when conversion is not possible.
 */
extern si::Time
to_unix_time (xf::nmea::GPSDate const&, xf::nmea::GPSTimeOfDay const&);

extern std::string
to_string (xf::nmea::GPSDate const& date);

extern std::string
to_string (xf::nmea::GPSTimeOfDay const& time);

} // namespace xf::nmea

#endif

