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

#ifndef XEFIS__MODULES__SIMULATION__VIRTUAL_MODEM_H__INCLUDED
#define XEFIS__MODULES__SIMULATION__VIRTUAL_MODEM_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/simulation/devices/antenna.h>
#include <xefis/support/sockets/socket_changed.h>

// Neutrino:
#include <neutrino/blob.h>

// Standard:
#include <cstddef>


namespace nu = neutrino;
namespace si = nu::si;


struct VirtualModemParameters
{
	xf::ModuleParameters	module_parameters;
	xf::sim::Antenna&		antenna;
	si::Frequency			frequency;
	si::Frequency			bandwidth;
	si::Frequency			bitrate;
	std::size_t				tx_buffer_size;
	si::Power				tx_power;
	si::Power				rx_sensitivity;
};


/**
 * Communicates with another modem.
 */
class VirtualModem: public xf::Module
{
  public:
	xf::ModuleIn<std::string>	send				{ this, "send" };

	xf::ModuleOut<std::string>	receive				{ this, "receive" };
	xf::ModuleOut<si::Power>	rssi				{ this, "rssi" };

	// Total number of bytes dropped due to TX buffer overflow:
	xf::ModuleOut<uint64_t>		tx_bytes_dropped	{ this, "tx bytes dropped" };

  private:
	using Signals = std::vector<std::string>;

  public:
	// Ctor
	explicit
	VirtualModem (VirtualModemParameters const&);

	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	void
	send_signals (xf::Cycle const&);

	void
	receive_signals();

	[[nodiscard]]
	double
	frequency_response (si::Frequency frequency) const;

  private:
	xf::sim::Antenna&	_antenna;
	si::Frequency		_frequency;
	si::Frequency		_bandwidth;
	si::Frequency		_bitrate;
	std::size_t			_tx_buffer_capacity;
	si::Power			_tx_power;
	si::Power			_rx_sensitivity;
	nu::Blob			_tx_buffer;

	// Accumulated number of transmittable bits carried across cycles:
	double				_tx_bit_budget	{ 0.0 };

	xf::SocketChanged	_send_changed	{ this->send };
};

#endif
