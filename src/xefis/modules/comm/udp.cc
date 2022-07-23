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

// Neutrino:
#include <neutrino/qt/qdom.h>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "udp.h"


UDP::UDP (xf::Logger const& logger, std::string_view const& instance):
	UDP_IO (instance),
	_logger (logger.with_scope (std::string (kLoggerScope) + "#" + instance))
{
	if (_io.tx_udp_host && _io.tx_udp_port)
		_tx = std::make_unique<QUdpSocket>();

	if (_io.rx_udp_host && _io.rx_udp_port)
	{
		_rx = std::make_unique<QUdpSocket>();

		if (!_rx->bind (QHostAddress (*_io.rx_udp_host), *_io.rx_udp_port, QUdpSocket::ShareAddress))
			_logger << "Failed to bind to address " << _io.rx_udp_host->toStdString() << ":" << *_io.rx_udp_port << std::endl;

		QObject::connect (_rx.get(), SIGNAL (readyRead()), this, SLOT (got_udp_packet()));
	}
}


void
UDP::process (xf::Cycle const&)
{
	if (_io.send && _send_changed.serial_changed())
	{
		std::string const& data = *_io.send;
		QByteArray blob (data.data(), data.size());

		if (_io.tx_interference)
			interfere (blob);

		if (_tx && _io.tx_udp_host && _io.tx_udp_port)
			_tx->writeDatagram (blob.data(), blob.size(), QHostAddress (*_io.tx_udp_host), *_io.tx_udp_port);
	}
}


void
UDP::got_udp_packet()
{
	while (_rx->hasPendingDatagrams())
	{
		auto datagram_size = _rx->pendingDatagramSize();

		if (_received_datagram.size() < datagram_size)
			_received_datagram.resize (datagram_size);

		_rx->readDatagram (_received_datagram.data(), datagram_size, nullptr, nullptr);
	}

	if (_io.rx_interference)
		interfere (_received_datagram);

	_io.receive = std::string (_received_datagram.data(), neutrino::to_unsigned (_received_datagram.size()));
}


void
UDP::interfere (QByteArray& blob)
{
	if (rand() % 3 == 0)
	{
		// Erase random byte from the input sequence:
		int i = rand() % blob.size();
		blob.remove (i, 1);
	}
}

