/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
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
#include "standard_atmosphere.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/constants.h>

// Neutrino:
#include <neutrino/math/field.h>

// Standard:
#include <cstddef>


namespace xf {
namespace {

struct InternationalStandardAtmosphereParams
{
	si::Pressure	pressure;
	si::Temperature	temperature;
	si::Density		density;
};


// Data taken from <https://en.wikipedia.org/wiki/International_Standard_Atmosphere>
// Maps geometric AMSL altitude to atmosphere parameters.
std::map<si::Length, InternationalStandardAtmosphereParams> const&
international_standard_atmosphere_map()
{
	static std::map<si::Length, InternationalStandardAtmosphereParams> const kInternationalStandardAtmosphereMap {
		{ -0.61_km,  { 108'900.00_Pa,   254.15_K, 1.2985_kgpm3      } },
		{  0.0_km,   { 101'325.00_Pa,   288.15_K, 1.2250_kgpm3      } },
		{ 11.0_km,   {  22'632.10_Pa,   216.65_K, 0.36391_kgpm3     } },
		{ 20.0_km,   {   5'474.89_Pa,   216.65_K, 0.08803_kgpm3     } },
		{ 32.0_km,   {     868.02_Pa,   228.65_K, 0.01322_kgpm3     } },
		{ 47.0_km,   {     110.91_Pa,   270.65_K, 0.0020_kgpm3      } },
		{ 51.0_km,   {      66.939_Pa,  270.65_K, 0.00086_kgpm3     } },
		{ 71.0_km,   {       3.9564_Pa, 214.65_K, 0.000064211_kgpm3 } },
		{ 84.852_km, {       0.3734_Pa, 186.87_K, 8.0510e-6_kgpm3   } },
	};

	return kInternationalStandardAtmosphereMap;
}


Field<si::Length, si::Temperature> const&
international_standard_atmosphere_temperature()
{
	static Field<si::Length, si::Temperature> const kInternationalStandardAtmosphereTemperature {
		{ -0.61_km,  254.15_K },
		{  0.0_km,   288.15_K },
		{ 11.0_km,   216.65_K },
		{ 20.0_km,   216.65_K },
		{ 32.0_km,   228.65_K },
		{ 47.0_km,   270.65_K },
		{ 51.0_km,   270.65_K },
		{ 71.0_km,   214.65_K },
		{ 84.852_km, 186.87_K },
	};

	return kInternationalStandardAtmosphereTemperature;
}


// Map of temperature <-> dynamic viscosity taken from
// <http://www.engineeringtoolbox.com/air-absolute-kinematic-viscosity-d_601.html>
Field<si::Temperature, si::DynamicViscosity> const&
air_temperature_to_dynamic_viscosity()
{
	static Field<si::Temperature, si::DynamicViscosity> const kAirTemperatureToDynamicViscosity {
		{  -40_degF, 157.591e-7_Pas },
		{  -20_degF, 159.986e-7_Pas },
		{    0_degF, 157.591e-7_Pas },
		{   10_degF, 164.776e-7_Pas },
		{   20_degF, 167.650e-7_Pas },
		{   30_degF, 171.482e-7_Pas },
		{   40_degF, 172.440e-7_Pas },
		{   50_degF, 176.272e-7_Pas },
		{   60_degF, 179.625e-7_Pas },
		{   70_degF, 182.978e-7_Pas },
		{   80_degF, 184.894e-7_Pas },
		{   90_degF, 186.810e-7_Pas },
		{  100_degF, 188.726e-7_Pas },
		{  120_degF, 192.558e-7_Pas },
		{  140_degF, 197.827e-7_Pas },
		{  160_degF, 202.138e-7_Pas },
		{  180_degF, 207.886e-7_Pas },
		{  200_degF, 215.071e-7_Pas },
		{  300_degF, 238.063e-7_Pas },
		{  400_degF, 250.996e-7_Pas },
		{  500_degF, 277.820e-7_Pas },
		{  750_degF, 326.199e-7_Pas },
		{ 1000_degF, 376.015e-7_Pas },
		{ 1500_degF, 455.050e-7_Pas },
	};

	return kAirTemperatureToDynamicViscosity;
}


inline si::TemperatureGradient
standard_temperature_gradient (std::map<si::Length, InternationalStandardAtmosphereParams>::const_iterator const& lower_layer,
							   std::map<si::Length, InternationalStandardAtmosphereParams>::const_iterator const& upper_layer)
{
	if (lower_layer == upper_layer)
		return 0_K / 0_m;

	auto const delta_temperature = upper_layer->second.temperature - lower_layer->second.temperature;
	auto const delta_altitude = upper_layer->first - lower_layer->first;

	return delta_temperature / delta_altitude;
}

} // namespace


Air
StandardAtmosphere::air_at (SpaceVector<si::Length, ECEFSpace> const& position) const
{
	return air_at_radius (abs (position));
}


Air
StandardAtmosphere::air_at_radius (si::Length radius) const
{
	return air_at_amsl (radius - kEarthMeanRadius);
}


Air
StandardAtmosphere::air_at_amsl (si::Length geometric_altitude_amsl) const
{
	Air air;
	air.density = standard_density (geometric_altitude_amsl);
	air.pressure = standard_pressure (geometric_altitude_amsl);
	air.temperature = standard_temperature (geometric_altitude_amsl);
	air.dynamic_viscosity = dynamic_air_viscosity (air.temperature);
	air.speed_of_sound = speed_of_sound (air.temperature);
	return air;
}


SpaceVector<si::Velocity, ECEFSpace>
StandardAtmosphere::wind_at (SpaceVector<si::Length, ECEFSpace> const&) const
{
	// TODO model it.
	return { 0_mps, 0_mps, 0_mps };
}


AtmosphereState<ECEFSpace>
StandardAtmosphere::state_at (SpaceVector<si::Length, ECEFSpace> const& position) const
{
	return {
		air_at (position),
		wind_at (position),
	};
}


si::Density
standard_density (si::Length geometric_altitude_amsl)
{
	// Using formulas from <https://en.wikipedia.org/wiki/Barometric_formula>

	auto const& atmmap = international_standard_atmosphere_map();

	geometric_altitude_amsl = std::clamp (geometric_altitude_amsl, atmmap.begin()->first, atmmap.rbegin()->first);

	auto upper_layer_it = atmmap.upper_bound (geometric_altitude_amsl);

	if (upper_layer_it == atmmap.begin())
		upper_layer_it = std::next (atmmap.begin());
	else if (upper_layer_it == atmmap.end())
		--upper_layer_it;

	auto const lower_layer_it = std::prev (upper_layer_it);
	auto const& lower_layer = lower_layer_it->second;
	auto const h = geometric_altitude_amsl;
	auto const hb = lower_layer_it->first;
	auto const Lb = standard_temperature_gradient (lower_layer_it, upper_layer_it);
	auto const Tb = lower_layer.temperature;
	auto const rb = lower_layer.density;

	if (abs (Lb) > 0.0_K / 1_m)
	{
		auto const p = Tb / (Tb + Lb * (h - hb));
		auto const e = 1 + (kStdGravitationalAcceleration * kAirMolarMass / (kUniversalGasConstant * Lb));

		return rb * std::pow (p, e);
	}
	else
		return rb * std::exp (-kStdGravitationalAcceleration * kAirMolarMass * (h - hb) / (kUniversalGasConstant * Tb));
}


si::Pressure
standard_pressure (si::Length geometric_altitude_amsl)
{
	// Using formulas from <https://en.wikipedia.org/wiki/Barometric_formula>

	auto const& atmmap = international_standard_atmosphere_map();

	geometric_altitude_amsl = std::clamp (geometric_altitude_amsl, atmmap.begin()->first, atmmap.rbegin()->first);

	auto upper_layer_it = atmmap.upper_bound (geometric_altitude_amsl);

	if (upper_layer_it == atmmap.begin())
		upper_layer_it = std::next (atmmap.begin());
	else if (upper_layer_it == atmmap.end())
		--upper_layer_it;

	auto const lower_layer_it = std::prev (upper_layer_it);
	auto const& lower_layer = lower_layer_it->second;
	auto const h = geometric_altitude_amsl;
	auto const hb = lower_layer_it->first;
	auto const Lb = standard_temperature_gradient (lower_layer_it, upper_layer_it);
	auto const Tb = lower_layer.temperature;
	auto const Pb = lower_layer.pressure;

	if (abs (Lb) > 0.0_K / 1_m)
	{
		auto const p = Tb / (Tb + Lb * (h - hb));
		auto const e = kStdGravitationalAcceleration * kAirMolarMass / (kUniversalGasConstant * Lb);

		return Pb * std::pow (p, e);
	}
	else
		return Pb * std::exp (-kStdGravitationalAcceleration * kAirMolarMass * (h - hb) / (kUniversalGasConstant * Tb));
}


si::Temperature
standard_temperature (si::Length geometric_altitude_amsl)
{
	return international_standard_atmosphere_temperature().extrapolated_value (geometric_altitude_amsl);
}


si::TemperatureGradient
standard_temperature_gradient (si::Length geometric_altitude_amsl)
{
	// Find gradient in international_standard_atmosphere_temperature().

	auto const& atmmap = international_standard_atmosphere_map();

	geometric_altitude_amsl = std::clamp (geometric_altitude_amsl, atmmap.begin()->first, atmmap.rbegin()->first);

	auto upper_layer_it = atmmap.upper_bound (geometric_altitude_amsl);

	if (upper_layer_it == atmmap.end())
		return 0_K / 1_m;
	else
	{
		if (upper_layer_it == atmmap.begin())
			upper_layer_it = std::next (atmmap.begin());

		auto const lower_layer_it = std::prev (upper_layer_it);

		return standard_temperature_gradient (lower_layer_it, upper_layer_it);
	}
}


si::DynamicViscosity
dynamic_air_viscosity (si::Temperature temperature)
{
	return air_temperature_to_dynamic_viscosity().extrapolated_value (temperature);
}

} // namespace xf

