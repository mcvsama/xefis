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

#ifndef XEFIS__MODULES__COMM__UDP_H__INCLUDED
#define XEFIS__MODULES__COMM__UDP_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/sockets/socket_changed.h>

// Neutrino:
#include <neutrino/logger.h>

// Qt:
#include <QtNetwork/QUdpSocket>

// Standard:
#include <cstddef>


class UDP: public xf::Module
{
  public:
	/*
	 * Input
	 */

	xf::ModuleIn<std::string>	send				{ this, "send" };

	/*
	 * Output
	 */

	xf::ModuleOut<std::string>	receive				{ this, "receive" };

  public:
	struct Address
	{
		std::string	host;
		uint32_t	port;
	};

	struct Parameters
	{
		std::optional<Address>	rx_udp_address;
		std::optional<Address>	tx_udp_address;
		// Whether to randomly interfere with transmitted data:
		bool					rx_interference	{ false };
		// Whether to randomly interfere with received data:
		bool					tx_interference	{ false }; // TODO std::optional<float> tx_interference_probability;
	};

  private:
	static constexpr char kLoggerScope[] = "mod::UDP";

  public:
	// Ctor
	explicit
	UDP (xf::ProcessingLoop&, Parameters, xf::Logger const&, std::string_view const instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

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
	Parameters						_parameters;
	xf::Logger						_logger;
	QByteArray						_received_datagram;
	xf::SocketChanged				_send_changed	{ this->send };
	QHostAddress					_tx_qhostaddress;
	// Destroy first to disconnect signals:
	std::unique_ptr<QUdpSocket>		_rx;
	std::unique_ptr<QUdpSocket>		_tx;
};

#endif
