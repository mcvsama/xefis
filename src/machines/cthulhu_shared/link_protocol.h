/* vim:ts=4
 *
 * Copyleft 2008…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__CTHULHU_SHARED__LINK_PROTOCOL_H__INCLUDED
#define XEFIS__MACHINES__CTHULHU_SHARED__LINK_PROTOCOL_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/modules/comm/link.h>


class CthulhuGCS_Tx_LinkProtocol: public LinkProtocol
{
  public:
	// Ctor
	template<class IO>
		explicit
		CthulhuGCS_Tx_LinkProtocol (IO&);
};


class CthulhuGCS_Rx_LinkProtocol: public LinkProtocol
{
  public:
	// Ctor
	template<class IO>
		explicit
		CthulhuGCS_Rx_LinkProtocol (IO&);
};


template<class IO>
	CthulhuGCS_Tx_LinkProtocol::CthulhuGCS_Tx_LinkProtocol (IO& io):
		LinkProtocol ({
			envelope (Magic ({ 0xe4, 0x40 }), {
				signature (NonceBytes (8), SignatureBytes (12), Key ({ 0x88, 0x99, 0xaa, 0xbb }), {
					property<2> (io.stick_elevator, Retained (false)),
					property<2> (io.stick_ailerons, Retained (false)),
					property<2> (io.rudder_pedals, Retained (false)),
					property<2> (io.throttle_left, Retained (false)),
					property<2> (io.throttle_right, Retained (false)),
				}),
			}),
			envelope (Magic ({ 0xa3, 0x80 }), SendEvery (1000), SendOffset (0), {
				signature (NonceBytes (8), SignatureBytes (4), Key ({ 0x55, 0x37, 0x12, 0xf9 }), {
					bitfield ({
						bitfield_property (io.test_bool, Retained (false), false),
						bitfield_property (io.test_uint, Bits (4), Retained (false), 0UL),
					}),
				}),
			}),
		})
	{ }


template<class IO>
	CthulhuGCS_Rx_LinkProtocol::CthulhuGCS_Rx_LinkProtocol (IO& io):
		LinkProtocol({
			envelope (Magic ({ 0xe4, 0x40 }), {
				signature (NonceBytes (8), SignatureBytes (12), Key ({ 0x87, 0x11, 0x65, 0xa4 }), {
					property<8> (io.home_latitude, Retained (false)),
					property<8> (io.home_longitude, Retained (false)),
				}),
			}),
		})
	{ }

#endif

