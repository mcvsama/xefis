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

// Xefis:
#include <xefis/test/test.h>
#include <xefis/utility/numeric.h>


namespace xf {
namespace test {

static xf::RuntimeTest t1 ("numeric: integral()", []{
	using namespace xf::test_asserts;

	auto int1 = integral (static_cast<double (*)(double)> (std::sin), { -5.3, +12.0 }, 1e-5);

	verify_equal_with_epsilon ("integrated sin() is correct", int1, -0.28948, 1e-5);
});

} // namespace test
} // namespace xf

