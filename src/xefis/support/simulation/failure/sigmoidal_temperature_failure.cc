/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
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
#include "sigmoidal_temperature_failure.h"

// Standard:
#include <cstddef>
#include <cmath>


namespace xf {

SigmoidalTemperatureFailure::SigmoidalTemperatureFailure (si::Time const expected_normal_lifetime,
														  si::Temperature const normal_temperature,
														  si::Temperature const stress_temperature):
	_stress_temperature (stress_temperature)
{
	auto const normal_failure_probability_in_1s = 1_s / expected_normal_lifetime;

	_k = std::log (1.0 / normal_failure_probability_in_1s - 1) / (stress_temperature - normal_temperature);
}


bool
SigmoidalTemperatureFailure::should_fail (si::Temperature const temperature, si::Time const dt)
{
	return _random_distribution (_mersenne_twister) < failure_probability (temperature, dt);
}


double
SigmoidalTemperatureFailure::failure_probability (si::Temperature const temperature, si::Time const dt) const
{
	return dt / 1_s / (1.0 + std::exp (_k * (_stress_temperature - temperature)));
}

} // namespace xf

