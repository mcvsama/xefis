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
#include <iostream>
#include <iomanip>
#include <limits>
#include <cstdint>

// Xefis:
#include <xefis/core/property.h>
#include <xefis/test/test.h>


namespace xf {
namespace test {

using namespace test_asserts;
using namespace si::units;


template<class T>
	inline void
	do_t_1 ([[maybe_unused]] T&& fallback_value)
	{
#if 0
		// ConstantSource
		xf::PropertyOut<T> output { "/output" };
		xf::PropertyOut<T> output_with_owner { owner, "/output_with_owner" };

		xf::PropertyIn<T> input { owner, "/input" };
		xf::PropertyIn<T> input_with_fallback { owner, "/input_with_fallback", fallback_value };

		input << output;
		input_with_fallback << ConstantSource (xf::nil);

		// TODO tests !!input, *input, !!input_with_fallback, *input_with_fallback

		if constexpr (std::is_same_v<T, std::string>)
		{
			// TODO std::string-specific tests
		}
#endif
	}


static xf::RuntimeTest t_1 ("xf::Property transferring values", []{
	do_t_1<bool> (true);
	do_t_1<int64_t> (1337);
	do_t_1<float64_t> (0.125);
	do_t_1<std::string> ("fallback-value");
	do_t_1<uint_fast8_t> (8);
});

} // namespace test
} // namespace xf

