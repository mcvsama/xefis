/* vim:ts=4
 *
 * Copyleft 2017  Michał Gawron
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
#include "udp_transceiver.h"
#include "udp_transceiver_widget.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/qt/qstring.h>

// Lib:
#include <boost/endian/conversion.hpp>
#include <gsl/gsl_util>

// Qt:
#include <QWidget>

// Standard:
#include <cmath>
#include <cstddef>
#include <memory>
#include <random>
#include <ranges>


using namespace nu::si::literals;


UDPTransceiver::UDPTransceiver (xf::ProcessingLoop& loop, Parameters const parameters, nu::Logger const& logger, std::string_view const instance):
	Module (loop, instance),
	_parameters (parameters),
	_logger (logger.with_context (std::string (kLoggerScope) + "#" + instance)),
	_bandwidth_accounting ({
		.received_bandwidth = xf::BandwidthSampler ({ parameters.bandwidth_measurement_interval, parameters.bandwidth_history_size }),
		.transmitted_bandwidth = xf::BandwidthSampler ({ parameters.bandwidth_measurement_interval, parameters.bandwidth_history_size }),
	})
{
	if (_parameters.tx_udp_address)
	{
		_tx_qhostaddress = QHostAddress (nu::to_qstring (_parameters.tx_udp_address->host));
		_tx = std::make_unique<QUdpSocket>();
	}

	if (_parameters.rx_udp_address)
	{
		_rx = std::make_unique<QUdpSocket>();

		if (!_rx->bind (QHostAddress (nu::to_qstring (_parameters.rx_udp_address->host)), _parameters.rx_udp_address->port, QUdpSocket::ShareAddress))
			_logger << std::format ("Failed to bind to address {}:{}\n", _parameters.rx_udp_address->host, _parameters.rx_udp_address->port);

		QObject::connect (_rx.get(), &QUdpSocket::readyRead, [this] { got_udp_packet(); });
	}
}


UDPTransceiver::BandwidthSnapshot
UDPTransceiver::bandwidth_snapshot() const
{
	return {
		.received_samples = { std::from_range, _bandwidth_accounting.received_bandwidth.samples() },
		.transmitted_samples = { std::from_range, _bandwidth_accounting.transmitted_bandwidth.samples() },
	};
}


void
UDPTransceiver::process (xf::Cycle const& cycle)
{
	_bandwidth_accounting.received_bandwidth.flush (cycle.update_time());
	_bandwidth_accounting.transmitted_bandwidth.flush (cycle.update_time());

	if (_bandwidth_accounting.pending_received_bytes > 0u)
	{
		_bandwidth_accounting.received_bandwidth.record_bytes (_bandwidth_accounting.pending_received_bytes, cycle.update_time());
		_bandwidth_accounting.pending_received_bytes = 0u;
	}

	if (_tx && _parameters.tx_udp_address)
	{
		if (this->send)
		{
			std::string const& data = *this->send;
			QByteArray blob (data.data(), gsl::narrow<qsizetype> (data.size()));

			if (_parameters.tx_interference)
				interfere (blob);

			auto const written = _tx->writeDatagram (blob.data(), blob.size(), _tx_qhostaddress, _parameters.tx_udp_address->port);

			if (written > 0)
				_bandwidth_accounting.transmitted_bandwidth.record_bytes (gsl::narrow<std::size_t> (written), cycle.update_time());
		}
	}
}


QWidget*
UDPTransceiver::configurator_widget()
{
	if (!_configurator_widget)
		_configurator_widget = new UDPTransceiverWidget (*this);

	return _configurator_widget;
}


void
UDPTransceiver::got_udp_packet()
{
	std::size_t received_bytes = 0u;

	while (_rx->hasPendingDatagrams())
	{
		auto datagram_size = _rx->pendingDatagramSize();
		_received_datagram.resize (datagram_size);
		_rx->readDatagram (_received_datagram.data(), datagram_size, nullptr, nullptr);
		received_bytes += gsl::narrow<std::size_t> (datagram_size);
	}

	if (received_bytes > 0u)
		_bandwidth_accounting.pending_received_bytes += received_bytes;

	if (_parameters.rx_interference)
		interfere (_received_datagram);

	this->receive = std::string (_received_datagram.data(), nu::to_unsigned (_received_datagram.size()));
}


void
UDPTransceiver::interfere (QByteArray& blob)
{
	if (!blob.isEmpty() && rand() % 3 == 0)
	{
		// Erase random byte from the input sequence:
		int i = rand() % blob.size();
		blob.remove (i, 1);
	}
}
