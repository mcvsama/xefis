/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>
#include <memory>
#include <random>

// Qt:
#include <QtXml/QDomElement>

// Lib:
#include <boost/endian/conversion.hpp>
#include <gsl/gsl_util>

// Neutrino:
#include <neutrino/qt/qdom.h>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "udp.h"


UDP::UDP (xf::ProcessingLoop& loop, Parameters const parameters, xf::Logger const& logger, std::string_view const& instance):
	Module (loop, instance),
	_parameters (parameters),
	_logger (logger.with_context (std::string (kLoggerScope) + "#" + instance))
{
	if (_parameters.tx_udp_address)
	{
		_tx_qhostaddress = QHostAddress (QString::fromStdString (_parameters.tx_udp_address->host));
		_tx = std::make_unique<QUdpSocket>();
	}

	if (_parameters.rx_udp_address)
	{
		_rx = std::make_unique<QUdpSocket>();

		if (!_rx->bind (QHostAddress (QString::fromStdString (_parameters.rx_udp_address->host)), _parameters.rx_udp_address->port, QUdpSocket::ShareAddress))
			_logger << std::format ("Failed to bind to address {}:{}\n", _parameters.rx_udp_address->host, _parameters.rx_udp_address->port);

		QObject::connect (_rx.get(), &QUdpSocket::readyRead, [this] { got_udp_packet(); });
	}
}


void
UDP::process (xf::Cycle const&)
{
	if (_tx && _parameters.tx_udp_address)
	{
		if (this->send)
		{
			std::string const& data = *this->send;
			QByteArray blob (data.data(), gsl::narrow<qsizetype> (data.size()));

			if (_parameters.tx_interference)
				interfere (blob);

			_tx->writeDatagram (blob.data(), blob.size(), _tx_qhostaddress, _parameters.tx_udp_address->port);
		}
	}
}


void
UDP::got_udp_packet()
{
	while (_rx->hasPendingDatagrams())
	{
		auto datagram_size = _rx->pendingDatagramSize();
		_received_datagram.resize (datagram_size);
		_rx->readDatagram (_received_datagram.data(), datagram_size, nullptr, nullptr);
	}

	if (_parameters.rx_interference)
		interfere (_received_datagram);

	this->receive = std::string (_received_datagram.data(), neutrino::to_unsigned (_received_datagram.size()));
}


void
UDP::interfere (QByteArray& blob)
{
	if (!blob.isEmpty() && rand() % 3 == 0)
	{
		// Erase random byte from the input sequence:
		int i = rand() % blob.size();
		blob.remove (i, 1);
	}
}

