/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
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
#include "temperature_sensor.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/various_inertia_tensors.h>
#include <xefis/support/shapes/various_materials.h>
#include <xefis/support/shapes/various_shapes.h>

// Standard:
#include <cstddef>


namespace xf::sim {

TemperatureSensor::TemperatureSensor (Atmosphere const& atmosphere, TemperatureSensorParameters const& params):
	Body (MassMoments<BodyCOM>()),
	_atmosphere (atmosphere),
	_recovery_factor (params.recovery_factor),
	_nose_position (params.dimensions.x(), 0_m, 0_m)
{
	auto const material = make_material ({ 0x6e, 0xb8, 0x6d });
	auto shape = make_centered_cube_shape (params.dimensions, material);
	// Make the back of the sensor the origin:
	shape.translate ({ 0.5 * params.dimensions.x(), 0_m, 0_m });
	set_shape (shape);
	// TODO add cone to the back of the sensor

	auto const inertia_tensor_at_com = make_cuboid_inertia_tensor<BodyCOM> (params.mass, math::coordinate_system_cast<void, void, BodyOrigin, void> (params.dimensions));
	// The inertia tensor we get is centered at space origin, but the sensor is not - it has origin at space origin. Therefore we need to shift the
	// inertia tensor like it was viewed from the origin of the temperature sensor, not from its COM.
	auto const offset = SpaceLength<BodyCOM> { 0.5 * params.dimensions.x(), 0_m, 0_m };
	auto const inertia_tensor_at_origin = inertia_tensor_com_to_point (params.mass, inertia_tensor_at_com, offset);
	// From that we can build MassMomentsAtArm which then can be auto converted to MassMoments when passed to set_mass_moments():
	set_mass_moments (MassMomentsAtArm<BodyCOM> (params.mass, offset, inertia_tensor_at_origin));
}


si::Temperature
TemperatureSensor::static_temperature() const
{
	compute_static_temperature();
	return *_static_temperature.load();
}


si::Temperature
TemperatureSensor::measured_temperature() const
{
	auto measured_temperature = _measured_temperature.lock();

	if (!*measured_temperature)
	{
		compute_static_temperature();
		compute_stagnation_temperature();
		// r(θ) is a recovery factor function; for simplicity use cos(θ) is used. It's actually not correct but in reality it depends on the probe shape and all,
		// and needs to be figured out experimentally or via computer simulations, so we can't make good general prediction here.
		// Formula:
		//   T_measured = T_static + r(θ)⋅(T_stagnation − T_static)
		auto const static_temperature = _static_temperature->value();
		auto const stagnation_temperature = _stagnation_temperature->value();
		*measured_temperature = static_temperature + _recovery_factor * (stagnation_temperature - static_temperature);
	}

	return measured_temperature->value();
}


si::Temperature
TemperatureSensor::stagnation_temperature() const
{
	compute_stagnation_temperature();
	return *_stagnation_temperature.load();
}


void
TemperatureSensor::evolve ([[maybe_unused]] si::Time dt)
{
	_static_temperature->reset();
	_stagnation_temperature->reset();
	_measured_temperature->reset();
}


void
TemperatureSensor::compute_static_temperature() const
{
	auto static_temperature = _static_temperature.lock();

	if (!*static_temperature)
		*static_temperature = _atmosphere.temperature_at (sampling_position());
}


void
TemperatureSensor::compute_stagnation_temperature() const
{
	auto stagnation_temperature = _stagnation_temperature.lock();

	if (!*stagnation_temperature)
	{
		compute_static_temperature();

		auto const Cp = 1005_J / (1_kg * 1_K);
		// T_stagnation is what sensor measures at the stagnation point if probe is mounted ideally (into the wind).
		// Formula:
		//   T_stagnation = T_static + v² / (2 * Cp), where Cp for air ~= 1005 J/(kg⋅K)
		auto const air_velocity = _atmosphere.wind_velocity_at (sampling_position());
		auto const sensor_velocity = coordinate_system_cast<ECEFSpace, void, WorldSpace, void> (velocity_moments<WorldSpace>().velocity());
		auto const sensor_velocity_relative_to_air = sensor_velocity - air_velocity;
		auto const sensor_normal_vector = coordinate_system_cast<ECEFSpace, void, WorldSpace, void> (placement().body_coordinates().x_axis()); // `sensor_normal_vector` is normalized.
		// This projection is simplification because in reality the relationship between mounting angle + wind direction and resulting temperature is
		// non-linear:
		auto const effective_sensor_velocity = projection_onto_normalized (sensor_velocity_relative_to_air, sensor_normal_vector);
		*stagnation_temperature = _static_temperature->value() + nu::square (abs (effective_sensor_velocity)) / (2 * Cp);
	}
}

} // namespace xf::sim

