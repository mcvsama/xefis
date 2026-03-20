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
#include "virtual_modem.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <optional>

using namespace nu::si::literals;


VirtualModem::VirtualModem (VirtualModemParameters const& params):
	Module (params.module_parameters),
	_antenna (params.antenna),
	_frequency (params.frequency),
	_bandwidth (params.bandwidth),
	_bitrate (params.bitrate),
	_tx_buffer_capacity (params.tx_buffer_size),
	_tx_power (params.tx_power),
	_rx_sensitivity (params.rx_sensitivity)
{
	if (_frequency <= 0_Hz)
		throw nu::InvalidArgument ("virtual modem frequency must be positive");

	if (_bandwidth <= 0_Hz)
		throw nu::InvalidArgument ("virtual modem bandwidth must be positive");

	if (_bitrate <= 0_Hz)
		throw nu::InvalidArgument ("virtual modem bitrate must be positive");

	_tx_buffer.reserve (params.tx_buffer_size);
	_antenna.set_recording_enabled (true);

	this->tx_bytes_dropped = 0;
}


void
VirtualModem::process (xf::Cycle const& cycle)
{
	send_signals (cycle);
	receive_signals();
}


void
VirtualModem::send_signals (xf::Cycle const& cycle)
{
	if (_tx_buffer.empty())
		_tx_bit_budget = 0.0;

	if (_send_changed.serial_changed())
	{
		auto const payload = this->send.value_or ("");

		if (!payload.empty())
		{
			auto const free_space = _tx_buffer_capacity - std::min (_tx_buffer.size(), _tx_buffer_capacity);
			auto const bytes_to_append = std::min (payload.size(), free_space);

			_tx_buffer.append (payload.begin(), payload.begin() + nu::to_signed (bytes_to_append));

			if (auto const dropped_bytes = payload.size() - bytes_to_append; dropped_bytes > 0)
				this->tx_bytes_dropped = this->tx_bytes_dropped.value_or (0) + dropped_bytes;
		}
	}

	_tx_bit_budget += double (cycle.intended_update_dt() * _bitrate);

	auto const bytes_to_send = std::min<std::size_t> (_tx_buffer.size(), _tx_bit_budget / 8.0);

	if (bytes_to_send > 0)
	{
		_antenna.emit_signal ({
			.time = cycle.update_time(),
			.power = _tx_power,
			.frequency = _frequency,
			.bandwidth = _bandwidth,
			.payload = std::string (_tx_buffer.begin(), _tx_buffer.begin() + nu::to_signed (bytes_to_send)),
		});
		_tx_buffer.erase (0, bytes_to_send);
		_tx_bit_budget -= 8.0 * bytes_to_send;
	}
}


void
VirtualModem::receive_signals()
{
	this->receive = xf::nil;

	auto strongest_received_power = std::optional<si::Power>();

	for (auto const& signal: _antenna.take_recorded_signals())
	{
		// Simplified receiver model: attenuate only by center-frequency mismatch.
		// We intentionally ignore the transmitted signal bandwidth and spectral
		// shape to keep the modem simulation cheap; decode and RSSI are based on
		// this attenuated total power.
		auto const received_power = signal.power * frequency_response (signal.frequency);

		if (!strongest_received_power || received_power > *strongest_received_power)
			strongest_received_power = received_power;

		if (received_power > _rx_sensitivity)
			this->receive = this->receive.value_or ("") + signal.payload;
	}

	if (strongest_received_power)
		this->rssi = *strongest_received_power;
	else
		this->rssi = xf::nil;
}


double
VirtualModem::frequency_response (si::Frequency const frequency) const
{
	auto const offset = abs (frequency - _frequency);
	auto const x = double (offset / _bandwidth);

	// Treat modem bandwidth as Gaussian full width at half maximum.
	return std::exp (-4.0 * std::numbers::ln2_v<double> * nu::square (x));
}
