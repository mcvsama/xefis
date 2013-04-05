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

#ifndef XEFIS__UTILITY__UNIT_PARSER_H__INCLUDED
#define XEFIS__UTILITY__UNIT_PARSER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QString>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

/**
 * Indicates that wrong unit has been used for requested value type.
 */
class InvalidUnit: public Exception
{
  public:
	InvalidUnit (const char* message);
};


/**
 * Indicates that the program was unable to parse value.
 * Format should be value, space, unit, eg. "12 kt".
 */
class UnparsableValue: public Exception
{
  public:
	UnparsableValue (const char* message);
};


class UnitParser
{
  public:
	/**
	 * Parse angle value.
	 * \throw	InvalidUnit if unit isn't an angle unit.
	 */
	static Angle
	parse_angle (QString const&);

	/**
	 * Parse frequency value.
	 * \throw	InvalidUnit if unit isn't a frequency unit.
	 */
	static Frequency
	parse_frequency (QString const&);

	/**
	 * Parse length value.
	 * \throw	InvalidUnit if unit isn't a length unit.
	 */
	static Length
	parse_length (QString const&);

	/**
	 * Parse pressure value.
	 * \throw	InvalidUnit if unit isn't a pressure unit.
	 */
	static Pressure
	parse_pressure (QString const&);

	/**
	 * Parse speed value.
	 * \throw	InvalidUnit if unit isn't a speed unit.
	 */
	static Speed
	parse_speed (QString const&);

	/**
	 * Parse time value.
	 * \throw	InvalidUnit if unit isn't a time unit.
	 */
	static Time
	parse_time (QString const&);

  private:
	/**
	 * Return a pair of <value, unit>.
	 */
	static std::pair<double, QString>
	parse (QString const&);
};


inline
InvalidUnit::InvalidUnit (const char* message):
	Exception (message)
{ }


inline
UnparsableValue::UnparsableValue (const char* message):
	Exception (message)
{ }


inline Angle
UnitParser::parse_angle (QString const& str)
{
	auto p = parse (str);

	if (p.second == "deg")
		return p.first * 1_deg;
	else if (p.second == "rad")
		return p.first * 1_rad;

	throw InvalidUnit();
}


inline Frequency
UnitParser::parse_frequency (QString const& str)
{
	auto p = parse (str);

	if (p.second == "hz")
		return p.first * 1_Hz;
	else if (p.second == "khz")
		return p.first * 1_kHz;
	else if (p.second == "mhz")
		return p.first * 1_MHz;

	throw InvalidUnit();
}


Length
parse_length (QString const& str)
{
	auto p = parse (str);

	if (p.second == "m")
		return p.first * 1_m;
	else if (p.second == "km")
		return p.first * 1_km;
	else if (p.second == "ft")
		return p.first * 1_ft;
	else if (p.second == "nm")
		return p.first * 1_nm;
	else if (p.second == "mil")
		return p.first * 1_mil;

	throw InvalidUnit();
}


Pressure
parse_pressure (QString const& str)
{
	auto p = parse (str);

	if (p.second == "psi")
		return p.first * 1_psi;
	else if (p.second == "hpa")
		return p.first * 1_hPa;
	else if (p.second == "inhg")
		return p.first * 1_inHg;

	throw InvalidUnit();
}


Speed
parse_speed (QString const& str)
{
	auto p = parse (str);

	if (p.second == "kt")
		return p.first * 1_kt;
	else if (p.second == "kph")
		return p.first * 1_kph;
	else if (p.second == "fpm")
		return p.first * 1_fpm;

	throw InvalidUnit();
}


Time
parse_time (QString const& str)
{
	auto p = parse (str);

	if (p.second == "us")
		return p.first * 1_us;
	else if (p.second == "ms")
		return p.first * 1_ms;
	else if (p.second == "s")
		return p.first * 1_s;
	else if (p.second == "min")
		return p.first * 1_min;
	else if (p.second == "h")
		return p.first * 1_h;

	throw InvalidUnit();
}


inline std::pair<double, QString>
UnitParser::parse (QString const& str)
{
	int p = str.indexOf (' ');
	if (p == -1)
		throw UnparsableValue();
	return { str.left (p), p.mid (p + 1).toLower() };
}

} // namespace Xefis

#endif

