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
#include <xefis/utility/datatable2d.h>


namespace xf::test {
namespace {

RuntimeTest t1 ("Datatable2D<>", []{
	Datatable2D<double, double> d ({
		{ 0.0, 0.0 },
		{ 1.0, 10.0 },
		{ 0.0, 0.0 },
		{ 1.0, 10.0 },
		{ 2.0, 20.0 },
		{ 3.0, 10.0 },
		{ 4.0, 0.0 },
	});

	test_asserts::verify ("domain().min() is correct", d.domain().min() == 0.0);
	test_asserts::verify ("domain().max() is correct", d.domain().max() == 4.0);

	test_asserts::verify ("codomain().min() is correct", d.codomain().min() == 0.0);
	test_asserts::verify ("codomain().max() is correct", d.codomain().max() == 20.0);

	test_asserts::verify_equal_with_epsilon ("average ({ -2.0, 2.0 }) is correct", d.average ({ -2.0, 2.0 }), 0.0, 0.001);
});

} // namespace
} // namespace xf::test

