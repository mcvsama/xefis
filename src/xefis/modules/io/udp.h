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

#ifndef XEFIS__MODULES__IO__UDP_H__INCLUDED
#define XEFIS__MODULES__IO__UDP_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtNetwork/QUdpSocket>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/module_io.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/v2/actions.h>


class UDP_IO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<QString>			tx_udp_host			{ this, "tx_udp_host" }; // TODO express that this is optional
	v2::Setting<int>				tx_udp_port			{ this, "tx_udp_port" };
	v2::Setting<bool>				tx_interference		{ this, "tx_interference", false };
	v2::Setting<QString>			rx_udp_host			{ this, "rx_udp_host" }; // TODO OptionalSetting
	v2::Setting<int>				rx_udp_port			{ this, "rx_udp_port" };
	v2::Setting<bool>				rx_interference		{ this, "rx_interference", false };

	/*
	 * Input
	 */

	v2::PropertyIn<std::string>		send				{ this, "/send" };

	/*
	 * Output
	 */

	v2::PropertyOut<std::string>	receive				{ this, "/receive" };
};


class UDP:
	public QObject,
	public v2::Module<UDP_IO>
{
	Q_OBJECT

  public:
	// Ctor
	explicit
	UDP (std::unique_ptr<UDP_IO> module_io, std::string const& instance = {});

	// Module API
	void
	process (v2::Cycle const&) override;

  private slots:
	/**
	 * Called whenever there's data ready to be read from socket.
	 */
	void
	got_udp_packet();

  private:
	/**
	 * Interfere with packets for testing purposes.
	 */
	void
	interfere (QByteArray& blob);

  private:
	QByteArray						_received_datagram;
	Unique<QUdpSocket>				_tx;
	Unique<QUdpSocket>				_rx;
	v2::PropChanged<std::string>	_send_changed	{ io.send };
};

#endif
