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

#ifndef XEFIS__MODULES__COMM__UDP_TRANSCEIVER_H__INCLUDED
#define XEFIS__MODULES__COMM__UDP_TRANSCEIVER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/properties/has_configurator_widget.h>
#include <xefis/support/stats/bandwidth_sampler.h>

// Neutrino:
#include <neutrino/logger.h>

// Qt:
#include <QPointer>
#include <QtNetwork/QUdpSocket>

// Standard:
#include <cstddef>
#include <vector>


namespace nu = neutrino;
namespace si = nu::si;
using namespace si::literals;


class UDPTransceiver:
	public xf::Module,
	public xf::HasConfiguratorWidget
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
		si::Time				bandwidth_measurement_interval	{ 1_s };
		std::size_t				bandwidth_history_size			{ 300u };

		// Whether to randomly interfere with transmitted data:
		bool					rx_interference					{ false };

		// Whether to randomly interfere with received data:
		bool					tx_interference					{ false }; // TODO std::optional<float> tx_interference_probability;
	};

	using Bandwidth = xf::BandwidthSampler::Bandwidth;

	struct BandwidthSnapshot
	{
		std::vector<Bandwidth>	received_samples;
		std::vector<Bandwidth>	transmitted_samples;
	};

	struct BandwidthAccounting
	{
		xf::BandwidthSampler	received_bandwidth;
		xf::BandwidthSampler	transmitted_bandwidth;
		std::size_t				pending_received_bytes	{ 0u };
	};

  private:
	static constexpr char kLoggerScope[] = "mod::UDPTransceiver";

  public:
	// Ctor
	explicit
	UDPTransceiver (xf::ProcessingLoop&, Parameters, nu::Logger const&, std::string_view const instance = {});

	[[nodiscard]]
	BandwidthSnapshot
	bandwidth_snapshot() const;

	// Module API
	void
	process (xf::Cycle const&) override;

	// HasConfiguratorWidget API
	[[nodiscard]]
	QWidget*
	configurator_widget() override;

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
	Parameters					_parameters;
	nu::Logger					_logger;
	QByteArray					_received_datagram;
	QHostAddress				_tx_qhostaddress;
	BandwidthAccounting			_bandwidth_accounting;
	// Cached non-owning pointer; the host Qt container owns and deletes the widget.
	QPointer<QWidget>			_configurator_widget;
	// Last on list to be destroyed first to disconnect signals:
	std::unique_ptr<QUdpSocket>	_rx;
	std::unique_ptr<QUdpSocket>	_tx;
};

#endif
