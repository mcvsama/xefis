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

// Xefis:
#include <xefis/support/earth/air/standard_atmosphere.h>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Standard:
#include <cstddef>
#include <cmath>


namespace xf::test {
namespace {

template<class Function>
	void test_nans_for_altitude (Function&& function)
	{
		auto const check = [&function](si::Length altitude) {
			test_asserts::verify ("returned NaN for altitude " + to_string (altitude),
								  !isnan (function (altitude)));
		};

		check (0_m);

		for (si::Length altitude = -1000_km; altitude < 1000_km; altitude += 100_m)
			check (altitude);

		for (si::Length altitude = 1000_km; altitude < 1000000_km; altitude += 100_km)
			check (altitude);
	}


AutoTest t_standard_density ("xf::standard_density() doesn't return NaNs", []{
	test_nans_for_altitude (standard_density);
});


AutoTest t_standard_pressure ("xf::standard_pressure() doesn't return NaNs", []{
	test_nans_for_altitude (standard_pressure);
});


AutoTest t_standard_temperature ("xf::standard_temperature() doesn't return NaNs", []{
	test_nans_for_altitude (standard_temperature);
});


AutoTest t_standard_temperature_gradient ("xf::standard_temperature_gradient() doesn't return NaNs", []{
	test_nans_for_altitude (standard_temperature_gradient);
});


// TODO Make tests for:
// TODO   dynamic_air_viscosity (si::Temperature);
// TODO   speed_of_sound (si::Temperature static_air_temperature)
// TODO   density_altitude (si::Length pressure_altitude, si::Temperature static_air_temperature)
// TODO   true_airspeed (si::Velocity indicated_airspeed, si::Length density_altitude)
// TODO   indicated_airspeed (si::Velocity true_airspeed, si::Length density_altitude)

} // namespace
} // namespace xf::test

