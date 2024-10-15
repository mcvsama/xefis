/* vim:ts=4
 *
 * Copyleft 2020  Michał Gawron
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
#include "gps.h"
#include "exceptions.h"
#include "nmea.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>
#include <neutrino/stdexcept.h>

// Standard:
#include <cstddef>
#include <format>
#include <string>


namespace xf::nmea {

using std::to_string;


static inline unsigned int
mknum (char c10, char c01)
{
	return digit_from_ascii (c10) * 10u
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
		seconds_fraction = neutrino::parse<double> (gps_time.substr (6));
	}
	catch (InvalidFormat& e)
	{
		std::throw_with_nested (InvalidFormat ("invalid format of GPS time-of-day"));
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
		std::throw_with_nested (InvalidFormat ("invalid format of GPS date"));
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
		this->tracked_satellites = neutrino::parse<unsigned int> (val());
	}
	catch (ParseException&)
	{ }

	// Horizontal dilution of position:
	if (!read_next())
		return;

	try {
		this->hdop = neutrino::parse<double> (val());
	}
	catch (ParseException&)
	{ }

	// Altitude above mean sea level (in meters):
	if (!read_next())
		return;

	try {
		this->altitude_amsl = 1_m * neutrino::parse<double> (val());
	}
	catch (ParseException&)
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
		this->geoid_height = 1_m * neutrino::parse<double> (val());
	}
	catch (ParseException&)
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
		this->dgps_last_update_time = 1_s * neutrino::parse<double> (val());
	}
	catch (ParseException&)
	{ }

	// DGPS station identifier:
	if (!read_next())
		return;

	try {
		this->dgps_station_id = neutrino::parse<std::decay_t<decltype (*this->dgps_station_id)>> (val());
	}
	catch (ParseException&)
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
	for (uint32_t i = 0u; i < 12u; ++i)
	{
		if (!read_next())
			return;

		try {
			if (!val().empty())
				this->satellites[i] = neutrino::parse<unsigned int> (val());
		}
		catch (ParseException&)
		{ }
	}

	// PDOP:
	if (!read_next())
		return;

	try {
		this->pdop = neutrino::parse<double> (val());
	}
	catch (ParseException&)
	{ }

	// HDOP:
	if (!read_next())
		return;

	try {
		this->hdop = neutrino::parse<double> (val());
	}
	catch (ParseException&)
	{ }

	// VDOP:
	if (!read_next())
		return;

	try {
		this->vdop = neutrino::parse<double> (val());
	}
	catch (ParseException&)
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
		this->ground_speed = 1_kt * neutrino::parse<double> (val());
	}
	catch (ParseException&)
	{ }

	// Track angle in degrees True:
	if (!read_next())
		return;

	try {
		this->track_true = 1_deg * neutrino::parse<double> (val());
	}
	catch (ParseException&)
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
		this->magnetic_variation = 1_deg * neutrino::parse<double> (val());
	}
	catch (ParseException&)
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
	// TODO Use C++20 designated initializers with enum:
	static std::array<std::string, 9> const fix_quality_strings {
		"invalid",
		"GPS",
		"DGPS",
		"PPS",
		"RTK",
		"float RTK",
		"estimated",
		"manual input mode",
		"simulated mode",
	};

	int code_int = static_cast<int> (code);

	if (code_int < 0 || 8 < code_int)
		code_int = static_cast<int> (GPSFixQuality::Invalid);

	return fix_quality_strings[static_cast<size_t> (code_int)];
}


si::Time
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
	return std::format ("{:d}-{:02d}-{:02d}", date.year, date.month, date.day);
}

std::string
to_string (xf::nmea::GPSTimeOfDay const& time)
{
	return std::format ("{:02d}:{:02d}:{:09f}", time.hours, time.minutes, time.seconds + time.seconds_fraction);
}

} // namespace xf::nmea

