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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/string.h>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Standard:
#include <cstddef>


namespace xf::test {
namespace {

namespace test_asserts = nu::test_asserts;


nu::AutoTest t_parse_hex_string("parse_hex_string()", []{
	// Valid inputs:
	test_asserts::verify_equal ("Single byte 'A5'", parse_hex_string ("A5"), Blob { 0xA5 });
	test_asserts::verify_equal ("Two bytes 'A5:B2'", parse_hex_string ("A5:B2"), Blob { 0xA5, 0xB2 });
	test_asserts::verify_equal ("Three bytes '00:FF:7A'", parse_hex_string ("00:FF:7A"), Blob { 0x00, 0xFF, 0x7A });
	test_asserts::verify_equal ("Four bytes 'AB:CD:EF:12'", parse_hex_string ("AB:CD:EF:12"), Blob {0xAB, 0xCD, 0xEF, 0x12});
	test_asserts::verify_equal ("Empty string", parse_hex_string (""), Blob());

	// Invalid inputs (should throw nu::Exception):
	test_asserts::verify_throws ("Odd hex digit count 'A5:B'", []{ parse_hex_string ("A5:B"); });
	test_asserts::verify_throws ("Invalid character 'G1:23:45'", []{ parse_hex_string ("G1:23:45"); });
	test_asserts::verify_throws ("Double colon '12:34:56::78'", []{ parse_hex_string ("12:34:56::78"); });
	test_asserts::verify_throws ("Trailing incomplete byte '12:34:5'", []{ parse_hex_string ("12:34:5"); });
});


nu::AutoTest t_parse_color ("parse_color()", []{
	// Named colors:
	test_asserts::verify_equal ("Named color 'red'", parse_color ("red"), QColor(Qt::red));
	test_asserts::verify_equal ("Named color 'green'", parse_color ("green"), QColor(Qt::green));
	test_asserts::verify_equal ("Named color 'blue'", parse_color ("blue"), QColor(Qt::blue));
	test_asserts::verify_equal ("Named color 'white'", parse_color ("white"), QColor(Qt::white));
	test_asserts::verify_equal ("Named color 'black'", parse_color ("black"), QColor(Qt::black));
	test_asserts::verify_equal ("Named color 'darkred'", parse_color ("darkred"), QColor(Qt::darkRed));
	test_asserts::verify_equal ("Named color 'darkblue'", parse_color ("darkblue"), QColor(Qt::darkBlue));
	test_asserts::verify_equal ("Named color 'cyan'", parse_color ("cyan"), QColor(Qt::cyan));
	test_asserts::verify_equal ("Named color 'darkcyan'", parse_color ("darkcyan"), QColor(Qt::darkCyan));
	test_asserts::verify_equal ("Named color 'magenta'", parse_color ("magenta"), QColor(Qt::magenta));
	test_asserts::verify_equal ("Named color 'darkmagenta'", parse_color ("darkmagenta"), QColor(Qt::darkMagenta));
	test_asserts::verify_equal ("Named color 'yellow'", parse_color ("yellow"), QColor(Qt::yellow));
	test_asserts::verify_equal ("Named color 'darkyellow'", parse_color ("darkyellow"), QColor(Qt::darkYellow));
	test_asserts::verify_equal ("Named color 'gray'", parse_color ("gray"), QColor(Qt::gray));
	test_asserts::verify_equal ("Named color 'darkgray'", parse_color ("darkgray"), QColor(Qt::darkGray));
	test_asserts::verify_equal ("Named color 'lightgray'", parse_color ("lightgray"), QColor(Qt::lightGray));

	// Case insensitivity:
	test_asserts::verify_equal ("Named color 'Red' (mixed case)", parse_color ("Red"), QColor(Qt::red));
	test_asserts::verify_equal ("Named color 'WHITE' (uppercase)", parse_color ("WHITE"), QColor(Qt::white));

	// Hex colors:
	test_asserts::verify_equal ("Hex color '#f00' -> #ff0000", parse_color ("#f00"), QColor(255, 0, 0));
	test_asserts::verify_equal ("Hex color '#ff0000'", parse_color ("#ff0000"), QColor(255, 0, 0));
	test_asserts::verify_equal ("Hex color '#00ff00' (Green)", parse_color ("#00ff00"), QColor(0, 255, 0));
	test_asserts::verify_equal ("Hex color '#0000ff' (Blue)", parse_color ("#0000ff"), QColor(0, 0, 255));

	test_asserts::verify_equal ("Hex color '#f00f' (RGBA) -> #ff0000ff", parse_color ("#f00f"), QColor(255, 0, 0, 255));
	test_asserts::verify_equal ("Hex color '#ff0000ff' (RRGGBBAA)", parse_color ("#ff0000ff"), QColor(255, 0, 0, 255));

	test_asserts::verify_equal ("Hex color '#abcdef'", parse_color ("#abcdef"), QColor(0xAB, 0xCD, 0xEF));

	// Invalid inputs:
	test_asserts::verify_equal ("Invalid hex '#12345' (wrong length)", parse_color ("#12345"), Qt::transparent);
	test_asserts::verify_equal ("Invalid hex '#GGG' (non-hex characters)", parse_color ("#GGG"), Qt::transparent);
	test_asserts::verify_equal ("Invalid hex '#1234567' (wrong length)", parse_color ("#1234567"), Qt::transparent);

	test_asserts::verify_equal ("Unknown color 'unknowncolor'", parse_color ("unknowncolor"), Qt::transparent);
	test_asserts::verify_equal ("Empty string", parse_color (""), Qt::transparent);
});

} // namespace
} // namespace xf::test

