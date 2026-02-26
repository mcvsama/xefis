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

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__TEMPERATURE_SENSOR_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__TEMPERATURE_SENSOR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/atmosphere/atmosphere.h>
#include <xefis/support/simulation/devices/temperature_sensor_widget.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/ui/observation_widget.h>

// Neutrino:
#include <neutrino/synchronized.h>

// Standard:
#include <cstddef>
#include <memory>
#include <optional>


namespace xf::sim {

struct TemperatureSensorParameters
{
	si::Mass				mass;

	// X is the front face of the sensor that ideally should point into the wind;
	// sampling point (air entrance) is at the {X, 0, 0} point in BodyOrigin space.
	SpaceLength<BodyOrigin>	dimensions;

	// Correction factor for returned measured temperature:
	float					recovery_factor	{ 1.0f };
};


class TemperatureSensor:
	public rigid_body::Body,
	public HasObservationWidget
{
  public:
	/**
	 * \param	Atmosphere
	 *			PrandtlTube can't outlive the Atmosphere.
	 */
	explicit
	TemperatureSensor (Atmosphere const&, TemperatureSensorParameters const&);

	/**
	 * Actual static air temperature read from the Atmosphere model.
	 */
	[[nodiscard]]
	si::Temperature
	static_temperature() const;

	/**
	 * Raw measured temperature. It's stagnation temperature corrected by the recovery_factor.
	 */
	[[nodiscard]]
	si::Temperature
	measured_temperature() const;

	/**
	 * Return the temperature as measured by the temperature sensor mounted on the moving aircraft at the stagnation point. Takes into account kinetic energy
	 * of the air, but not the temperature rise from the friction.
	 * air velocity.
	 */
	[[nodiscard]]
	si::Temperature
	stagnation_temperature() const;

	// HasObservationWidget API
	[[nodiscard]]
	std::unique_ptr<ObservationWidget>
	create_observation_widget() override
		{ return std::make_unique<TemperatureSensorWidget> (*this); }

	// Body API
	void
	evolve ([[maybe_unused]] si::Time dt) override;

  private:
	SpaceLength<ECEFSpace>
	sampling_position() const noexcept
		{ return coordinate_system_cast<ECEFSpace, void, WorldSpace, void> (placement().rotate_translate_to_base (origin_placement_in_com().rotate_translate_to_base (_nose_position))); }

	void
	compute_static_temperature() const;

	void
	compute_stagnation_temperature() const;

  private:
	Atmosphere const&											_atmosphere;
	float														_recovery_factor;
	// Position at which the air is sampled (nose (front) of the sensor):
	SpaceLength<BodyOrigin>										_nose_position;
	nu::Synchronized<std::optional<si::Temperature>> mutable	_static_temperature;
	nu::Synchronized<std::optional<si::Temperature>> mutable	_stagnation_temperature;
	nu::Synchronized<std::optional<si::Temperature>> mutable	_measured_temperature;
};

} // namespace xf::sim

#endif
