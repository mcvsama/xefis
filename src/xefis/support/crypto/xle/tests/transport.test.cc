/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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

// Neutrino:
#include <neutrino/test/auto_test.h>

// Xefis:
#include <xefis/support/crypto/xle/transport.h>


namespace xf::test {
namespace {

AutoTest t1 ("LossyCrypto: encryption and decryption", []{
	xf::crypto::xle::Transmitter tx (value_to_blob ("abcdefghijklmnop"));
	xf::crypto::xle::Receiver rx (value_to_blob ("abcdefghijklmnop"));

	Blob plain_text = value_to_blob ("plain text");
	Blob encrypted = tx.encrypt_packet (plain_text);
	Blob decrypted = rx.decrypt_packet (encrypted);

	test_asserts::verify ("decryption (1) works", decrypted == plain_text);

	plain_text = value_to_blob ("some other plain text that is longer than the AES key size");
	encrypted = tx.encrypt_packet (plain_text);
	decrypted = rx.decrypt_packet (encrypted);

	test_asserts::verify ("decryption (2) works", decrypted == plain_text);
});

} // namespace
} // namespace xf::test

