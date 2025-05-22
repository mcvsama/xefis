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


struct InputLinkParams
{
	std::optional<si::Time>		reacquire_after;
	std::optional<si::Time>		failsafe_after;
};


class InputLink: public xf::Module
{
  public:
	xf::ModuleIn<std::string>	link_input				{ this, "input" };

	xf::ModuleOut<bool>			link_valid				{ this, "link-valid" };
	xf::ModuleOut<int64_t>		link_failsafes			{ this, "failsafes" };
	xf::ModuleOut<int64_t>		link_reacquires			{ this, "reacquires" };
	// This is set by the LinkProtocol:
	xf::ModuleOut<int64_t>		link_error_bytes		{ this, "error-bytes" };
	xf::ModuleOut<int64_t>		link_valid_bytes		{ this, "valid-bytes" };
	// This is set by the LinkProtocol:
	xf::ModuleOut<int64_t>		link_valid_envelopes	{ this, "valid-envelopes" };

  private:
	static constexpr char kLoggerScope[] = "mod::InputLink";

  public:
	// Ctor
	explicit
	InputLink (xf::ProcessingLoop&, std::unique_ptr<LinkProtocol>, InputLinkParams const&, xf::Logger const&, std::string_view const& instance = {});

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
	std::unique_ptr<QTimer>			_failsafe_timer;
	std::unique_ptr<QTimer>			_reacquire_timer;
	Blob							_input_blob;
	std::unique_ptr<LinkProtocol>	_protocol;
	xf::SocketChanged				_input_changed		{ link_input };
	InputLinkParams					_params;
};

#endif

