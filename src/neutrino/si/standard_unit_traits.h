/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef NEUTRINO__SI__STANDARD_UNIT_TRAITS_H__INCLUDED
#define NEUTRINO__SI__STANDARD_UNIT_TRAITS_H__INCLUDED

// Standard:
#include <cstddef>

// Local:
#include "unit_traits.h"
#include "standard_units.h"
#include "additional_units.h"


namespace si {

/**
 * Return global symbol->DynamicUnit map.
 */
std::map<std::string, DynamicUnit> const&
units_map();


/**
 * Return global DynamicUnit->symbol map.
 */
std::map<DynamicUnit, std::string> const&
symbols_map();


/*
 * Unit traits for base SI units
 */


template<>
	struct UnitTraits<units::Dimensionless>: public DefaultUnitTraits
	{
		static std::string name()	{ return ""; }
		static std::string symbol()	{ return ""; }
	};


template<>
	struct UnitTraits<units::Meter>: public DefaultUnitTraits
	{
		static std::string name()	{ return "meter"; }
		static std::string symbol()	{ return "m"; }
	};


template<>
	struct UnitTraits<units::Kilogram>: public DefaultUnitTraits
	{
		static std::string name()	{ return "kilogram"; }
		static std::string symbol()	{ return "kg"; }
	};


template<>
	struct UnitTraits<units::Second>: public DefaultUnitTraits
	{
		static std::string name()	{ return "second"; }
		static std::string symbol()	{ return "s"; }
	};


template<>
	struct UnitTraits<units::Ampere>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Ampere"; }
		static std::string symbol()	{ return "A"; }
	};


template<>
	struct UnitTraits<units::Kelvin>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Kelvin"; }
		static std::string symbol()	{ return "K"; }
	};


template<>
	struct UnitTraits<units::Mole>: public DefaultUnitTraits
	{
		static std::string name()	{ return "mole"; }
		static std::string symbol()	{ return "mol"; }
	};


template<>
	struct UnitTraits<units::Candela>: public DefaultUnitTraits
	{
		static std::string name()	{ return "candela"; }
		static std::string symbol()	{ return "cd"; }
	};


template<>
	struct UnitTraits<units::Radian>: public DefaultUnitTraits
	{
		static std::string name()	{ return "radian"; }
		static std::string symbol()	{ return "rad"; }
	};


/*
 * Derived units
 */


template<>
	struct UnitTraits<units::Hertz>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Hertz"; }
		static std::string symbol()	{ return "Hz"; }

		// Becquerel (Bq) has the same exponents vector as Hertz.
	};


template<>
	struct UnitTraits<units::Steradian>: public DefaultUnitTraits
	{
		static std::string name()	{ return "steradian"; }
		static std::string symbol()	{ return "sr"; }
	};


template<>
	struct UnitTraits<units::Newton>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Newton"; }
		static std::string symbol()	{ return "N"; }
	};


template<>
	struct UnitTraits<units::Pascal>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Pascal"; }
		static std::string symbol()	{ return "Pa"; }
	};


template<>
	struct UnitTraits<units::Joule>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Joule"; }
		static std::string symbol()	{ return "J"; }
	};


template<>
	struct UnitTraits<units::Watt>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Watt"; }
		static std::string symbol()	{ return "W"; }
	};


template<>
	struct UnitTraits<units::Coulomb>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Coulomb"; }
		static std::string symbol()	{ return "C"; }
	};


template<>
	struct UnitTraits<units::Volt>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Volt"; }
		static std::string symbol()	{ return "V"; }
	};


template<>
	struct UnitTraits<units::Farad>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Farad"; }
		static std::string symbol()	{ return "F"; }
	};


template<>
	struct UnitTraits<units::Ohm>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Ohm"; }
		static std::string symbol()	{ return "Ω"; }
	};


template<>
	struct UnitTraits<units::Siemens>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Siemens"; }
		static std::string symbol()	{ return "S"; }
	};


template<>
	struct UnitTraits<units::Weber>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Weber"; }
		static std::string symbol()	{ return "Wb"; }
	};


template<>
	struct UnitTraits<units::Tesla>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Tesla"; }
		static std::string symbol()	{ return "T"; }
	};


template<>
	struct UnitTraits<units::Henry>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Henry"; }
		static std::string symbol()	{ return "H"; }
	};


template<>
	struct UnitTraits<units::Lumen>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Lumen"; }
		static std::string symbol()	{ return "lm"; }
	};


template<>
	struct UnitTraits<units::Lux>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Lux"; }
		static std::string symbol()	{ return "lx"; }
	};


template<>
	struct UnitTraits<units::Gray>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Gray"; }
		static std::string symbol()	{ return "Gy"; }
	};


template<>
	struct UnitTraits<units::Katal>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Katal"; }
		static std::string symbol()	{ return "kat"; }
	};


/*
 * Other - nonstandard units
 */


template<>
	struct UnitTraits<units::Kilometer>: public DefaultUnitTraits
	{
		static std::string name()	{ return "kilometer"; }
		static std::string symbol()	{ return "km"; }
	};


template<>
	struct UnitTraits<units::Centimeter>: public DefaultUnitTraits
	{
		static std::string name()	{ return "centimeter"; }
		static std::string symbol()	{ return "cm"; }
	};


template<>
	struct UnitTraits<units::Millimeter>: public DefaultUnitTraits
	{
		static std::string name()	{ return "millimeter"; }
		static std::string symbol()	{ return "mm"; }
	};


template<>
	struct UnitTraits<units::Tonne>: public DefaultUnitTraits
	{
		static std::string name()	{ return "tonne"; }
		static std::string symbol()	{ return "ton"; }
	};


template<>
	struct UnitTraits<units::Gram>: public DefaultUnitTraits
	{
		static std::string name()	{ return "gram"; }
		static std::string symbol()	{ return "gr"; }
	};


template<>
	struct UnitTraits<units::Milligram>: public DefaultUnitTraits
	{
		static std::string name()	{ return "milligram"; }
		static std::string symbol()	{ return "mg"; }
	};


template<>
	struct UnitTraits<units::Microgram>
	{
		static std::string name()	{ return "microgram"; }
		static std::string symbol()	{ return "µg"; }
		static std::vector<std::string> alternative_symbols() { return { "ug" }; }
	};


template<>
	struct UnitTraits<units::Hour>: public DefaultUnitTraits
	{
		static std::string name()	{ return "hour"; }
		static std::string symbol()	{ return "h"; }
	};


template<>
	struct UnitTraits<units::Minute>: public DefaultUnitTraits
	{
		static std::string name()	{ return "minute"; }
		static std::string symbol()	{ return "min"; }
	};


template<>
	struct UnitTraits<units::Millisecond>: public DefaultUnitTraits
	{
		static std::string name()	{ return "millisecond"; }
		static std::string symbol()	{ return "ms"; }
	};


template<>
	struct UnitTraits<units::Microsecond>
	{
		static std::string name()	{ return "microsecond"; }
		static std::string symbol()	{ return "µs"; }
		static std::vector<std::string> alternative_symbols() { return { "us" }; }
	};


template<>
	struct UnitTraits<units::Nanosecond>: public DefaultUnitTraits
	{
		static std::string name()	{ return "nanosecond"; }
		static std::string symbol()	{ return "ns"; }
	};


template<>
	struct UnitTraits<units::MilliAmpere>: public DefaultUnitTraits
	{
		static std::string name()	{ return "milliampere"; }
		static std::string symbol()	{ return "mA"; }
	};


template<>
	struct UnitTraits<units::MicroAmpere>
	{
		static std::string name()	{ return "microampere"; }
		static std::string symbol()	{ return "µA"; }
		static std::vector<std::string> alternative_symbols() { return { "uA" }; }
	};


template<>
	struct UnitTraits<units::Amperehour>: public DefaultUnitTraits
	{
		static std::string name()	{ return "amperehour"; }
		static std::string symbol()	{ return "Ah"; }
	};


template<>
	struct UnitTraits<units::MilliAmperehour>: public DefaultUnitTraits
	{
		static std::string name()	{ return "milliamperehour"; }
		static std::string symbol()	{ return "mAh"; }
	};


template<>
	struct UnitTraits<units::KiloNewton>: public DefaultUnitTraits
	{
		static std::string name()	{ return "kilonewton"; }
		static std::string symbol()	{ return "kN"; }
	};


template<>
	struct UnitTraits<units::MegaHertz>: public DefaultUnitTraits
	{
		static std::string name()	{ return "megahertz"; }
		static std::string symbol()	{ return "MHz"; }
	};


template<>
	struct UnitTraits<units::KiloHertz>: public DefaultUnitTraits
	{
		static std::string name()	{ return "kilohertz"; }
		static std::string symbol()	{ return "kHz"; }
	};


template<>
	struct UnitTraits<units::MegaWatt>: public DefaultUnitTraits
	{
		static std::string name()	{ return "megawatt"; }
		static std::string symbol()	{ return "MW"; }
	};


template<>
	struct UnitTraits<units::KiloWatt>: public DefaultUnitTraits
	{
		static std::string name()	{ return "kilowatt"; }
		static std::string symbol()	{ return "kW"; }
	};


template<>
	struct UnitTraits<units::MilliWatt>: public DefaultUnitTraits
	{
		static std::string name()	{ return "milliwatt"; }
		static std::string symbol()	{ return "mW"; }
	};


template<>
	struct UnitTraits<units::MicroWatt>
	{
		static std::string name()	{ return "microwatt"; }
		static std::string symbol()	{ return "µW"; }
		static std::vector<std::string> alternative_symbols() { return { "uW" }; }
	};


template<>
	struct UnitTraits<units::KiloPascal>: public DefaultUnitTraits
	{
		static std::string name()	{ return "kilopascal"; }
		static std::string symbol()	{ return "kPa"; }
	};


template<>
	struct UnitTraits<units::HectoPascal>: public DefaultUnitTraits
	{
		static std::string name()	{ return "hectopascal"; }
		static std::string symbol()	{ return "hPa"; }
	};


template<>
	struct UnitTraits<units::Foot>: public DefaultUnitTraits
	{
		static std::string name()	{ return "foot"; }
		static std::string symbol()	{ return "ft"; }
	};


template<>
	struct UnitTraits<units::Mile>: public DefaultUnitTraits
	{
		static std::string name()	{ return "mile"; }
		static std::string symbol()	{ return "mi"; }
	};


template<>
	struct UnitTraits<units::NauticalMile>: public DefaultUnitTraits
	{
		static std::string name()	{ return "nautical mile"; }
		static std::string symbol()	{ return "nmi"; }
	};


template<>
	struct UnitTraits<units::PoundMass>: public DefaultUnitTraits
	{
		static std::string name()	{ return "pound-mass"; }
		static std::string symbol()	{ return "lb"; }
	};


template<>
	struct UnitTraits<units::Gravity>: public DefaultUnitTraits
	{
		static std::string name()	{ return "gravity"; }
		static std::string symbol()	{ return "g"; }
	};


template<>
	struct UnitTraits<units::Rankine>: public DefaultUnitTraits
	{
		static std::string name()	{ return "Rankine"; }
		static std::string symbol()	{ return "Ra"; }
	};


template<>
	struct UnitTraits<units::Degree>
	{
		static std::string name()	{ return "degree"; }
		static std::string symbol()	{ return "°"; }
		static std::vector<std::string> alternative_symbols() { return { "deg" }; }
	};


template<>
	struct UnitTraits<units::InchOfMercury>: public DefaultUnitTraits
	{
		static std::string name()	{ return "inch of mercury"; }
		static std::string symbol()	{ return "inHg"; }
	};


template<>
	struct UnitTraits<units::KilometerPerHour>
	{
		static std::string name()	{ return "kilometer/hour"; }
		static std::string symbol()	{ return "km/h"; }
		static std::vector<std::string> alternative_symbols() { return { "kph" }; }
	};


template<>
	struct UnitTraits<units::FootPerSecond>
	{
		static std::string name()	{ return "foot/second"; }
		static std::string symbol()	{ return "ft/s"; }
		static std::vector<std::string> alternative_symbols() { return { "fps" }; }
	};


template<>
	struct UnitTraits<units::FootPerMinute>
	{
		static std::string name()	{ return "foot/minute"; }
		static std::string symbol()	{ return "ft/m"; }
		static std::vector<std::string> alternative_symbols() { return { "fpm" }; }
	};


template<>
	struct UnitTraits<units::Knot>
	{
		static std::string name()	{ return "knot"; }
		static std::string symbol()	{ return "kt"; }
		static std::vector<std::string> alternative_symbols() { return { "kn" }; }
	};


template<>
	struct UnitTraits<units::RotationPerMinute>: public DefaultUnitTraits
	{
		static std::string name()	{ return "rotations/minute"; }
		static std::string symbol()	{ return "RPM"; }
	};


template<>
	struct UnitTraits<units::Celsius>
	{
		static std::string name()	{ return "Celsius"; }
		static std::string symbol()	{ return "°C"; }
		static std::vector<std::string> alternative_symbols() { return { "degC" }; }
	};


template<>
	struct UnitTraits<units::Fahrenheit>
	{
		static std::string name()	{ return "Fahrenheit"; }
		static std::string symbol()	{ return "°F"; }
		static std::vector<std::string> alternative_symbols() { return { "degF" }; }
	};

} // namespace si

#endif

