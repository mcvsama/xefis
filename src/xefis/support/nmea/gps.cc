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

// Boost:
#include <boost/lexical_cast.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/mutex.h>
#include <xefis/utility/numeric.h>

// Local:
#include "gps.h"
#include "exceptions.h"
#include "nmea.h"


namespace xf::nmea {

using std::to_string;


static Mutex $fix_quality_strings_entry_mutex;


static inline unsigned int
mknum (uint8_t c10, uint8_t c01)
{
	return digit_from_ascii (c10) * 10
		 + digit_from_ascii (c01);
}


BadDateTime::BadDateTime (xf::nmea::GPSDate const& date, xf::nmea::GPSTimeOfDay const& time):
	Exception ("invalid GPS date '" + to_string (date) + "' or time '" + to_string (time) + "'")
{ }


UnsupportedSentenceType::UnsupportedSentenceType (std::string const& sentence):
	Exception ("unsupported sentence: '" + sentence + "'")
{ }


GPSTimeOfDay::GPSTimeOfDay (std::string const& gps_time)
{
	if (gps_time.size() < 6)
		throw InvalidFormat ("invalid format of GPS time-of-day: '" + gps_time + "'");

	try {
		hours = mknum (gps_time[0], gps_time[1]);
		minutes = mknum (gps_time[2], gps_time[3]);
		seconds = mknum (gps_time[4], gps_time[5]);
		seconds_fraction = boost::lexical_cast<double> ("0" + gps_time.substr (6));
	}
	catch (InvalidFormat& e)
	{
		throw InvalidFormat ("invalid format of GPS time-of-day", &e);
	}
}


GPSDate::GPSDate (std::string const& gps_date)
{
	if (gps_date.size() != 6)
		throw InvalidFormat ("invalid format of GPS date: '" + gps_date + "'");

	try {
		day = mknum (gps_date[0], gps_date[1]);
		month = mknum (gps_date[2], gps_date[3]);
		year = 2000 + mknum (gps_date[4], gps_date[5]);
	}
	catch (InvalidFormat& e)
	{
		throw InvalidFormat ("invalid format of GPS date", &e);
	}
}


GPGGA::GPGGA (std::string const& sentence):
	Sentence (sentence)
{
	if (!read_next() || val() != "GPGGA")
		throw InvalidType ("GPGGA", val());

	// Fix time (UTC):
	if (!read_next())
		return;
	if (!val().empty())
		this->fix_time = GPSTimeOfDay (val());

	// Latitude:
	if (!read_latitude (this->latitude))
		return;

	// Longitude:
	if (!read_longitude (this->longitude))
		return;

	// Fix quality:
	if (!read_next())
		return;
	if (val().size() == 1 && std::isdigit (val()[0]))
	{
		int fq = digit_from_ascii (val()[0]);
		// Integer range check:
		if (fq <= static_cast<int> (GPSFixQuality::Simulated) &&
			fq >= static_cast<int> (GPSFixQuality::Invalid))
		{
			this->fix_quality = static_cast<GPSFixQuality> (fq);
		}
	}

	// Number of tracked satellites:
	if (!read_next())
		return;
	try {
		this->tracked_satellites = boost::lexical_cast<unsigned int> (val());
	}
	catch (boost::bad_lexical_cast&)
	{ }

	// Horizontal dilution of position:
	if (!read_next())
		return;
	try {
		this->hdop = boost::lexical_cast<double> (val());
	}
	catch (boost::bad_lexical_cast&)
	{ }

	// Altitude above mean sea level (in meters):
	if (!read_next())
		return;
	try {
		this->altitude_amsl = 1_m * boost::lexical_cast<double> (val());
	}
	catch (boost::bad_lexical_cast&)
	{ }
	// Ensure that unit is 'M' (meters):
	if (!read_next())
	{
		this->altitude_amsl.reset();
		return;
	}

	if (val() != "M")
		this->altitude_amsl.reset();

	// Height above WGS84 geoid (in meters):
	if (!read_next())
		return;
	try {
		this->geoid_height = 1_m * boost::lexical_cast<double> (val());
	}
	catch (boost::bad_lexical_cast&)
	{ }
	// Ensure that unit is 'M' (meters):
	if (!read_next())
	{
		this->geoid_height.reset();
		return;
	}

	if (val() != "M")
		this->geoid_height.reset();

	// Time since last DGPS update (in seconds):
	if (!read_next())
		return;
	try {
		this->dgps_last_update_time = 1_s * boost::lexical_cast<double> (val());
	}
	catch (boost::bad_lexical_cast&)
	{ }

	// DGPS station identifier:
	if (!read_next())
		return;
	try {
		this->dgps_station_id = boost::lexical_cast<std::decay_t<decltype (*this->dgps_station_id)>> (val());
	}
	catch (boost::bad_lexical_cast&)
	{ }
}


bool
GPGGA::reliable_fix_quality() const noexcept
{
	if (fix_quality)
	{
		switch (*fix_quality)
		{
			case GPSFixQuality::GPS:
			case GPSFixQuality::DGPS:
			case GPSFixQuality::PPS:
			case GPSFixQuality::RTK:
			case GPSFixQuality::FloatRTK:
				return true;

			default:
				return false;
		}
	}
	else
		return false;
}


GPGSA::GPGSA (std::string const& sentence):
	Sentence (sentence)
{
	if (!read_next() || val() != "GPGSA")
		throw InvalidType ("GPGSA", val());

	// Fix selection (auto/manual):
	if (!read_next())
		return;

	if (val() == "M")
		this->fix_selection_mode = GPSFixSelectionMode::Manual;
	else if (val() == "A")
		this->fix_selection_mode = GPSFixSelectionMode::Auto;

	// Type of fix:
	if (!read_next())
		return;

	if (val() == "1")
		this->fix_mode = GPSFixMode::None;
	else if (val() == "2")
		this->fix_mode = GPSFixMode::Fix2D;
	else if (val() == "3")
		this->fix_mode = GPSFixMode::Fix3D;

	// PRNs of satellites used for the fix:
	for (int i = 0; i < 12; ++i)
	{
		if (!read_next())
			return;
		try {
			if (!val().empty())
				this->satellites[i] = boost::lexical_cast<unsigned int> (val());
		}
		catch (boost::bad_lexical_cast&)
		{ }
	}

	// PDOP:
	if (!read_next())
		return;
	try {
		this->pdop = boost::lexical_cast<double> (val());
	}
	catch (boost::bad_lexical_cast&)
	{ }

	// HDOP:
	if (!read_next())
		return;
	try {
		this->hdop = boost::lexical_cast<double> (val());
	}
	catch (boost::bad_lexical_cast&)
	{ }

	// VDOP:
	if (!read_next())
		return;
	try {
		this->vdop = boost::lexical_cast<double> (val());
	}
	catch (boost::bad_lexical_cast&)
	{ }
}


GPRMC::GPRMC (std::string const& sentence):
	Sentence (sentence)
{
	if (!read_next() || val() != "GPRMC")
		throw InvalidType ("GPRMC", val());

	// Fix time (UTC):
	if (!read_next())
		return;
	if (!val().empty())
		this->fix_time = GPSTimeOfDay (val());

	// Receiver status:
	if (!read_next())
		return;

	if (val() == "A")
		this->receiver_status = GPSReceiverStatus::Active;
	else if (val() == "v")
		this->receiver_status = GPSReceiverStatus::Void;

	// Latitude:
	if (!read_latitude (this->latitude))
		return;

	// Longitude:
	if (!read_longitude (this->longitude))
		return;

	// Ground-speed:
	if (!read_next())
		return;
	try {
		this->ground_speed = 1_kt * boost::lexical_cast<double> (val());
	}
	catch (boost::bad_lexical_cast&)
	{ }

	// Track angle in degrees True:
	if (!read_next())
		return;
	try {
		this->track_true = 1_deg * boost::lexical_cast<double> (val());
	}
	catch (boost::bad_lexical_cast&)
	{ }

	// Fix date:
	if (!read_next())
		return;
	if (!val().empty())
		this->fix_date = GPSDate (val());

	// Magnetic variation:
	if (!read_next())
		return;
	try {
		this->magnetic_variation = 1_deg * boost::lexical_cast<double> (val());
	}
	catch (boost::bad_lexical_cast&)
	{ }
	// East/West:
	if (!read_next())
	{
		this->magnetic_variation.reset();
		return;
	}

	if (val() == "W")
		this->magnetic_variation = -1 * *this->magnetic_variation;
	else if (val() != "E")
		this->magnetic_variation.reset();
}


std::string
to_string (GPSFixQuality code)
{
	// Must acquire lock before statically- and non-statically initializing static variables:
	auto lock = $fix_quality_strings_entry_mutex.acquire_lock();

	static std::array<std::string, 9> fix_quality_strings;
	static bool initialized = false;

	if (!initialized)
	{
		fix_quality_strings[0] = "invalid";
		fix_quality_strings[1] = "GPS";
		fix_quality_strings[2] = "DGPS";
		fix_quality_strings[3] = "PPS";
		fix_quality_strings[4] = "RTK";
		fix_quality_strings[5] = "float RTK";
		fix_quality_strings[6] = "estimated";
		fix_quality_strings[7] = "manual input mode";
		fix_quality_strings[8] = "simulated mode";
		initialized = true;
	}

	int code_int = static_cast<int> (code);
	if (code_int < 0 || 8 < code_int)
		code_int = static_cast<int> (GPSFixQuality::Invalid);
	return fix_quality_strings[code_int];
}


Time
to_unix_time (xf::nmea::GPSDate const& date, xf::nmea::GPSTimeOfDay const& time)
{
	struct tm timeinfo;
	timeinfo.tm_sec = time.seconds;
	timeinfo.tm_min = time.minutes;
	timeinfo.tm_hour = time.hours;
	timeinfo.tm_mday = date.day;
	timeinfo.tm_mon = date.month - 1;
	timeinfo.tm_year = date.year - 1900;
	timeinfo.tm_wday = -1;
	timeinfo.tm_yday = -1;
	timeinfo.tm_isdst = -1;
	time_t now = mktime (&timeinfo);
	if (now > 0)
		return 1_s * (1.0 * now + time.seconds_fraction);
	else
		throw BadDateTime (date, time);
}

std::string
to_string (xf::nmea::GPSDate const& date)
{
	// Need static_casting due to boost::format bug (?) that prints out uint8_t as characters even
	// though the format was explicitly %02d:
	return (boost::format ("%d-%02d-%02d")
			% static_cast<int> (date.year)
			% static_cast<int> (date.month)
			% static_cast<int> (date.day)).str();
}

std::string
to_string (xf::nmea::GPSTimeOfDay const& time)
{
	// Need static_casting due to boost::format bug (?) that prints out uint8_t as characters even
	// though the format was explicitly %02d:
	return (boost::format ("%02d:%02d:%02d.%d")
			% static_cast<int> (time.hours)
			% static_cast<int> (time.minutes)
			% static_cast<int> (time.seconds)
			% static_cast<int> (time.seconds_fraction)).str();
}

} // namespace xf::nmea

