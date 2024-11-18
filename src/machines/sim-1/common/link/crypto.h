/* vim:ts=4
 *
 * Copyleft 2008…2023  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIM_1__COMMON__LINK__CRYPTO_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__COMMON__LINK__CRYPTO_H__INCLUDED

// Machine:
#include <machines/sim-1/common/common.h>

// Neutrino:
#include <neutrino/si/si.h>

// Xefis:
#include <xefis/core/module.h>
#include <xefis/modules/comm/xle_transceiver.h>


namespace sim1 {

inline auto constinit kCryptoParams = xf::crypto::xle::Transceiver::CryptoParams {
	.master_signature_key		= { 0x00, 0x01, 0x02, 0x03 },
	.slave_signature_key		= { 0x0c, 0x0d, 0x0e, 0x0f },
	.authentication_secret		= { 0x01 },
	.data_encryption_secret		= { 0x02 },
	.seq_num_encryption_secret	= { 0x03 },
	.hmac_size					= 16,
	.max_time_difference		= 60_s,
};

} // namespace sim1

#endif

