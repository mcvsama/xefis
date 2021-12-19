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

// Boost:
#include <boost/random/random_device.hpp>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Xefis:
#include <xefis/support/crypto/xle/transport.h>


namespace xf::test {
namespace {

AutoTest t1 ("Xefis Lossy Encryption: encryption and decryption", []{
	Blob const key = value_to_blob ("abcdefghijklmnop");

	boost::random::random_device rnd;
	xf::crypto::xle::Transmitter tx (rnd, key);
	xf::crypto::xle::Receiver rx (key);

	Blob plain_text = value_to_blob ("");
	Blob encrypted = tx.encrypt_packet (plain_text);
	Blob decrypted = rx.decrypt_packet (encrypted);

	test_asserts::verify ("decryption (1) works", decrypted == plain_text);
	test_asserts::verify ("data margin is declared properly (1)", encrypted.size() - plain_text.size() == tx.data_margin());

	plain_text = value_to_blob ("some other plain text that is longer than the AES key size");
	encrypted = tx.encrypt_packet (plain_text);
	decrypted = rx.decrypt_packet (encrypted);

	test_asserts::verify ("decryption (2) works", decrypted == plain_text);
	test_asserts::verify ("data margin is declared properly (2)", encrypted.size() - plain_text.size() == tx.data_margin());
});

} // namespace
} // namespace xf::test

