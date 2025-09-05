/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__RADIO_MACHINE__RADIO_MODULES_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__RADIO_MACHINE__RADIO_MODULES_H__INCLUDED

// Xefis:
#include <xefis/core/sockets/module_in.h>
#include <xefis/core/sockets/module_out.h>
#include <xefis/modules/comm/link/helpers.h>
#include <xefis/modules/comm/link/input_link.h>
#include <xefis/modules/comm/link/output_link.h>
#include <xefis/modules/comm/udp.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

class RadioModules
{
  public:
	// Ctor
	explicit
	RadioModules (xf::ProcessingLoop&, nu::Logger const&);

  private:
	nu::Logger			_logger;
	xf::ProcessingLoop&	_loop;

  public:
	// TODO move this to VirtualRadioModules:
	UDP air_udp_link {
		_loop,
		UDP::Parameters {
			.rx_udp_address		= UDP::Address { "127.0.0.1", 9991 },
			.tx_udp_address		= UDP::Address { "127.0.0.1", 9990 },
			.rx_interference	= false,
			.tx_interference	= false,
		},
		_logger.with_context ("udp"),
		"udp",
	};
};

} // namespace sim1::aircraft

#endif
