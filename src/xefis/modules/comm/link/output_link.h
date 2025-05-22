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

#ifndef XEFIS__MODULES__COMM__LINK__OUTPUT_H__INCLUDED
#define XEFIS__MODULES__COMM__LINK__OUTPUT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/core/cycle.h>
#include <xefis/core/module.h>
#include <xefis/modules/comm/link/link_protocol.h>


namespace si = neutrino::si;
using namespace si::literals;


class OutputLink: public xf::Module
{
  public:
	xf::ModuleOut<std::string>	link_output				{ this, "output" };

  private:
	static constexpr char kLoggerScope[] = "mod::OutputLink";

  public:
	// Ctor
	explicit
	OutputLink (xf::ProcessingLoop&, std::unique_ptr<LinkProtocol>, si::Frequency send_frequency, xf::Logger const&, std::string_view const& instance = {});

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

  private slots:
	/**
	 * Called by output timer.
	 */
	void
	send_output();

  private:
	xf::Logger						_logger;
	std::unique_ptr<LinkProtocol>	_protocol;
	si::Time						_previous_update_time	{ 0_s };
	si::Time						_send_period;
	Blob							_output_blob;
};

#endif

