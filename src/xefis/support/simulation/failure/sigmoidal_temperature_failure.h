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

#ifndef XEFIS__SUPPORT__SIMULATION__FAILURE__SIGMOIDAL_TEMPERATURE_FAILURE_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__FAILURE__SIGMOIDAL_TEMPERATURE_FAILURE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/noncopyable.h>

// Standard:
#include <cstddef>
#include <random>


namespace xf {

/**
 * Component failure model, based on sigmoid function and component temperature.
 */
class SigmoidalTemperatureFailure
{
  public:
	/**
	 * Ctor
	 *
	 * \param	stress_temperature
	 *			Temperature at which probability of a failure within a 1 second is 0.5.
	 */
	explicit
	SigmoidalTemperatureFailure (si::Time expected_normal_lifetime, si::Temperature normal_temperature, si::Temperature stress_temperature);

	/**
	 * True if the device should fail within the specified Δt.
	 */
	[[nodiscard]]
	bool
	should_fail (si::Temperature, si::Time dt);

	/**
	 * Return probability of a failure within the specified Δt.
	 */
	[[nodiscard]]
	double
	failure_probability (si::Temperature, si::Time dt) const;

  private:
	si::Temperature						_stress_temperature;
	decltype(1 / 1_K)					_k;
	std::mt19937						_mersenne_twister		{ std::random_device{}() };
	std::uniform_real_distribution<>	_random_distribution	{ 0.0, 1.0 };
};

} // namespace xf

#endif

