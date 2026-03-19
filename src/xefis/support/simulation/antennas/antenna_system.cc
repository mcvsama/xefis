/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
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
#include "antenna_system.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/simulation/devices/antenna.h>

// Neutrino:
#include <neutrino/stdexcept.h>

// Standard:
#include <algorithm>
#include <cstddef>
#include <numbers>


namespace xf {

static Placement<WorldSpace, BodyOrigin>
body_origin_placement (sim::Antenna const& antenna)
{
	return Placement<WorldSpace, BodyOrigin> (
		antenna.origin<WorldSpace>(),
		antenna.placement().body_rotation() * antenna.origin_placement_in_com().body_rotation()
	);
}


AntennaSystem::AntennaSystem (si::Time const max_ttl):
	_max_ttl (max_ttl)
{ }


void
AntennaSystem::register_antenna (sim::Antenna& antenna)
{
	_antennas.insert (&antenna);

	for (auto& emission: _emissions)
		emission.receivers.insert (&antenna);
}


void
AntennaSystem::deregister_antenna (sim::Antenna& antenna)
{
	_antennas.erase (&antenna);

	for (auto& emission: _emissions)
		emission.receivers.erase (&antenna);
}


void
AntennaSystem::emit_signal (sim::Antenna& emitter, AntennaEmission const& antenna_emission)
{
	if (antenna_emission.frequency <= 0_Hz)
		throw nu::InvalidArgument ("emitted signal frequency must be positive");

	auto const emission = Emission {
		.emitter = emitter,
		.antenna_emission = antenna_emission,
		.placement = body_origin_placement (emitter),
		.receivers = _antennas,
	};
	_emissions.push_back (emission);
	_emissions.back().receivers.erase (&emitter);
}


void
AntennaSystem::process (si::Time const now)
{
	propagate_signals (now);
	cleanup_emissions (now);
}


void
AntennaSystem::propagate_signals (si::Time const now)
{
	for (auto& emission: _emissions)
	{
		auto& receivers = emission.receivers;

		for (auto receiver_it = receivers.begin(); receiver_it != receivers.end(); )
		{
			auto receiver = *receiver_it;
			auto const& antenna_emission = emission.antenna_emission;
			auto const receiver_placement = body_origin_placement (*receiver);
			auto const distance = abs (emission.placement.position() - receiver_placement.position());
			auto const time_due_to_distance = distance / kSpeedOfLight;
			auto const elapsed_time = now - emission.antenna_emission.time;

			if (elapsed_time >= time_due_to_distance)
			{
				auto const& tx_antenna_model = emission.emitter.model();
				auto const& rx_antenna_model = receiver->model();
				auto const frequency = antenna_emission.frequency;
				auto const wavelength = kSpeedOfLight / frequency;
				auto const tx_to_rx_direction = normalized_direction_or_zero (receiver_placement.position() - emission.placement.position());

				auto const tx_antenna_gain = tx_antenna_model.gain (frequency, emission.placement.rotate_to_body (+tx_to_rx_direction));
				auto const rx_antenna_gain = rx_antenna_model.gain (frequency, receiver_placement.rotate_to_body (-tx_to_rx_direction));
				auto const L_pol = polarization_coupling (
					tx_antenna_model,
					emission.placement,
					rx_antenna_model,
					receiver_placement,
					frequency,
					tx_to_rx_direction
				);
				auto const near_field_distance = wavelength / (4.0 * std::numbers::pi);
				auto const effective_distance = std::max (distance, near_field_distance);
				auto const k = nu::square (wavelength / (4.0 * std::numbers::pi * effective_distance));
				auto const received_signal_power = antenna_emission.power * tx_antenna_gain * rx_antenna_gain * L_pol * k;
				receiver->receive_signal ({
					.signal_power = received_signal_power,
					.payload = antenna_emission.payload,
				});

				receiver_it = emission.receivers.erase (receiver_it);
			}
			else
				++receiver_it;
		}
	}
}


void
AntennaSystem::cleanup_emissions (si::Time const now)
{
	_emissions.remove_if ([this, now] (Emission const& emission) {
		return emission.receivers.empty() || (now - emission.antenna_emission.time > _max_ttl);
	});
}

} // namespace xf
