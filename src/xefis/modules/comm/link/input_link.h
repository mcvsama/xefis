/* vim:ts=4
 *
 * Copyleft 2012…2023  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__COMM__LINK__INPUT_H__INCLUDED
#define XEFIS__MODULES__COMM__LINK__INPUT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/modules/comm/link/link_protocol.h>
#include <xefis/support/sockets/socket_changed.h>


namespace si = neutrino::si;


class InputLinkIO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Time>		reacquire_after			{ this, "reacquire_after", xf::BasicSetting::Optional };
	xf::Setting<si::Time>		failsafe_after			{ this, "failsafe_after", xf::BasicSetting::Optional };

	/*
	 * Input
	 */

	xf::ModuleIn<std::string>	link_input				{ this, "input" };

	/*
	 * Output
	 */

	xf::ModuleOut<bool>			link_valid				{ this, "link-valid" };
	xf::ModuleOut<int64_t>		link_failsafes			{ this, "failsafes" };
	xf::ModuleOut<int64_t>		link_reacquires			{ this, "reacquires" };
	xf::ModuleOut<int64_t>		link_error_bytes		{ this, "error-bytes" };
	xf::ModuleOut<int64_t>		link_valid_bytes		{ this, "valid-bytes" };
	xf::ModuleOut<int64_t>		link_valid_envelopes	{ this, "valid-envelopes" };

  public:
	using xf::Module::Module;
};


class InputLink:
	public QObject,
	public InputLinkIO
{
	Q_OBJECT

  private:
	static constexpr char kLoggerScope[] = "mod::InputLink";

  public:
	// Ctor
	explicit
	InputLink (std::unique_ptr<LinkProtocol>, xf::Logger const&, std::string_view const& instance = {});

	void
	process (xf::Cycle const&) override;

  private slots:
	/**
	 * Called by failsafe timer.
	 */
	void
	failsafe();

	/**
	 * Called by reacquire timer.
	 */
	void
	reacquire();

  private:
	xf::Logger						_logger;
	QTimer*							_failsafe_timer		{ nullptr };
	QTimer*							_reacquire_timer	{ nullptr };
	Blob							_input_blob;
	std::unique_ptr<LinkProtocol>	_protocol;
	xf::SocketChanged				_input_changed		{ link_input };
};

#endif

