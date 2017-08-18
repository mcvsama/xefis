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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>

// Local:
#include "udp.h"


UDP::UDP (std::unique_ptr<UDP_IO> module_io, std::string const& instance):
	Module (std::move (module_io), instance)
{
	if (io.tx_udp_host && io.tx_udp_port)
		_tx = std::make_unique<QUdpSocket>();

	if (io.rx_udp_host && io.rx_udp_port)
	{
		_rx = std::make_unique<QUdpSocket>();

		if (!_rx->bind (QHostAddress (*io.rx_udp_host), *io.rx_udp_port, QUdpSocket::ShareAddress))
			log() << "Failed to bind to address " << io.rx_udp_host->toStdString() << ":" << *io.rx_udp_port << std::endl;

		QObject::connect (_rx.get(), SIGNAL (readyRead()), this, SLOT (got_udp_packet()));
	}
}


void
UDP::process (v2::Cycle const&)
{
	if (io.send && _send_changed())
	{
		std::string const& data = *io.send;
		QByteArray blob (data.data(), data.size());

		if (io.tx_interference)
			interfere (blob);

		if (_tx && io.tx_udp_host && io.tx_udp_port)
			_tx->writeDatagram (blob.data(), blob.size(), QHostAddress (*io.tx_udp_host), *io.tx_udp_port);
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

	if (io.rx_interference)
		interfere (_received_datagram);

	io.receive = std::string (_received_datagram.data(), _received_datagram.size());
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

