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

// Standard:
#include <cstddef>

// Local:
#include "unit.h"
#include "standard_unit_traits.h"


namespace si {
namespace detail {
namespace {

template<class pUnit>
	void
	add_to_map (std::map<std::string, DynamicUnit>& map)
	{
		auto unit = pUnit::dynamic_unit();

		map.insert ({ UnitTraits<pUnit>::symbol(), unit });

		for (auto const& s: UnitTraits<pUnit>::alternative_symbols())
			map.insert ({ s, unit });
	}


std::map<std::string, DynamicUnit>
initialize_symbol2unit_map()
{
	std::map<std::string, DynamicUnit> result;

	detail::add_to_map<units::Dimensionless> (result);
	detail::add_to_map<units::Meter> (result);
	detail::add_to_map<units::Kilogram> (result);
	detail::add_to_map<units::Second> (result);
	detail::add_to_map<units::Ampere> (result);
	detail::add_to_map<units::Kelvin> (result);
	detail::add_to_map<units::Mole> (result);
	detail::add_to_map<units::Candela> (result);
	detail::add_to_map<units::Radian> (result);
	detail::add_to_map<units::Hertz> (result);
	detail::add_to_map<units::Steradian> (result);
	detail::add_to_map<units::Newton> (result);
	detail::add_to_map<units::Pascal> (result);
	detail::add_to_map<units::Joule> (result);
	detail::add_to_map<units::Watt> (result);
	detail::add_to_map<units::Coulomb> (result);
	detail::add_to_map<units::Volt> (result);
	detail::add_to_map<units::Farad> (result);
	detail::add_to_map<units::Ohm> (result);
	detail::add_to_map<units::Siemens> (result);
	detail::add_to_map<units::Weber> (result);
	detail::add_to_map<units::Tesla> (result);
	detail::add_to_map<units::Henry> (result);
	detail::add_to_map<units::Lumen> (result);
	detail::add_to_map<units::Lux> (result);
	detail::add_to_map<units::Becquerel> (result);
	detail::add_to_map<units::Gray> (result);
	detail::add_to_map<units::Katal> (result);
	detail::add_to_map<units::Kilometer> (result);
	detail::add_to_map<units::Centimeter> (result);
	detail::add_to_map<units::Millimeter> (result);
	detail::add_to_map<units::Tonne> (result);
	detail::add_to_map<units::Gram> (result);
	detail::add_to_map<units::Milligram> (result);
	detail::add_to_map<units::Microgram> (result);
	detail::add_to_map<units::Hour> (result);
	detail::add_to_map<units::Minute> (result);
	detail::add_to_map<units::Millisecond> (result);
	detail::add_to_map<units::Microsecond> (result);
	detail::add_to_map<units::Nanosecond> (result);
	detail::add_to_map<units::MilliAmpere> (result);
	detail::add_to_map<units::MicroAmpere> (result);
	detail::add_to_map<units::Amperehour> (result);
	detail::add_to_map<units::MilliAmperehour> (result);
	detail::add_to_map<units::KiloNewton> (result);
	detail::add_to_map<units::MegaHertz> (result);
	detail::add_to_map<units::KiloHertz> (result);
	detail::add_to_map<units::MegaWatt> (result);
	detail::add_to_map<units::KiloWatt> (result);
	detail::add_to_map<units::MilliWatt> (result);
	detail::add_to_map<units::MicroWatt> (result);
	detail::add_to_map<units::KiloPascal> (result);
	detail::add_to_map<units::HectoPascal> (result);
	detail::add_to_map<units::Foot> (result);
	detail::add_to_map<units::Mile> (result);
	detail::add_to_map<units::NauticalMile> (result);
	detail::add_to_map<units::PoundMass> (result);
	detail::add_to_map<units::Gravity> (result);
	detail::add_to_map<units::Rankine> (result);
	detail::add_to_map<units::Degree> (result);
	detail::add_to_map<units::InchOfMercury> (result);
	detail::add_to_map<units::KilometerPerHour> (result);
	detail::add_to_map<units::FootPerSecond> (result);
	detail::add_to_map<units::FootPerMinute> (result);
	detail::add_to_map<units::Knot> (result);
	detail::add_to_map<units::RotationPerMinute> (result);
	detail::add_to_map<units::Celsius> (result);
	detail::add_to_map<units::Fahrenheit> (result);

	return result;
}


template<class pUnit>
	void
	add_to_map (std::map<DynamicUnit, std::string>& map)
	{
		map.insert ({ pUnit::dynamic_unit(), UnitTraits<pUnit>::symbol() });
	}


std::map<DynamicUnit, std::string>
initialize_unit2symbol_map()
{
	std::map<DynamicUnit, std::string> result;

	detail::add_to_map<units::Dimensionless> (result);
	detail::add_to_map<units::Meter> (result);
	detail::add_to_map<units::Kilogram> (result);
	detail::add_to_map<units::Second> (result);
	detail::add_to_map<units::Ampere> (result);
	detail::add_to_map<units::Kelvin> (result);
	detail::add_to_map<units::Mole> (result);
	detail::add_to_map<units::Candela> (result);
	detail::add_to_map<units::Radian> (result);
	detail::add_to_map<units::Hertz> (result);
	detail::add_to_map<units::Steradian> (result);
	detail::add_to_map<units::Newton> (result);
	detail::add_to_map<units::Pascal> (result);
	detail::add_to_map<units::Joule> (result);
	detail::add_to_map<units::Watt> (result);
	detail::add_to_map<units::Coulomb> (result);
	detail::add_to_map<units::Volt> (result);
	detail::add_to_map<units::Farad> (result);
	detail::add_to_map<units::Ohm> (result);
	detail::add_to_map<units::Siemens> (result);
	detail::add_to_map<units::Weber> (result);
	detail::add_to_map<units::Tesla> (result);
	detail::add_to_map<units::Henry> (result);
	detail::add_to_map<units::Lumen> (result);
	detail::add_to_map<units::Lux> (result);
	detail::add_to_map<units::Becquerel> (result);
	detail::add_to_map<units::Gray> (result);
	detail::add_to_map<units::Katal> (result);
	detail::add_to_map<units::Kilometer> (result);
	detail::add_to_map<units::Centimeter> (result);
	detail::add_to_map<units::Millimeter> (result);
	detail::add_to_map<units::Tonne> (result);
	detail::add_to_map<units::Gram> (result);
	detail::add_to_map<units::Milligram> (result);
	detail::add_to_map<units::Microgram> (result);
	detail::add_to_map<units::Hour> (result);
	detail::add_to_map<units::Minute> (result);
	detail::add_to_map<units::Millisecond> (result);
	detail::add_to_map<units::Microsecond> (result);
	detail::add_to_map<units::Nanosecond> (result);
	detail::add_to_map<units::MilliAmpere> (result);
	detail::add_to_map<units::MicroAmpere> (result);
	detail::add_to_map<units::Amperehour> (result);
	detail::add_to_map<units::MilliAmperehour> (result);
	detail::add_to_map<units::KiloNewton> (result);
	detail::add_to_map<units::MegaHertz> (result);
	detail::add_to_map<units::KiloHertz> (result);
	detail::add_to_map<units::MegaWatt> (result);
	detail::add_to_map<units::KiloWatt> (result);
	detail::add_to_map<units::MilliWatt> (result);
	detail::add_to_map<units::MicroWatt> (result);
	detail::add_to_map<units::KiloPascal> (result);
	detail::add_to_map<units::HectoPascal> (result);
	detail::add_to_map<units::Foot> (result);
	detail::add_to_map<units::Mile> (result);
	detail::add_to_map<units::NauticalMile> (result);
	detail::add_to_map<units::PoundMass> (result);
	detail::add_to_map<units::Gravity> (result);
	detail::add_to_map<units::Rankine> (result);
	detail::add_to_map<units::Degree> (result);
	detail::add_to_map<units::InchOfMercury> (result);
	detail::add_to_map<units::KilometerPerHour> (result);
	detail::add_to_map<units::FootPerSecond> (result);
	detail::add_to_map<units::FootPerMinute> (result);
	detail::add_to_map<units::Knot> (result);
	detail::add_to_map<units::RotationPerMinute> (result);
	detail::add_to_map<units::Celsius> (result);
	detail::add_to_map<units::Fahrenheit> (result);

	return result;
}

} // namespace
} // namespace detail


std::map<std::string, DynamicUnit> const&
units_map()
{
	static std::map<std::string, DynamicUnit> symbol2unit_map = detail::initialize_symbol2unit_map();

	return symbol2unit_map;
}


std::map<DynamicUnit, std::string> const&
symbols_map()
{
	static std::map<DynamicUnit, std::string> unit2symbol_map = detail::initialize_unit2symbol_map();

	return unit2symbol_map;
}

} // namespace si

