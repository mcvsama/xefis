/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef NEUTRINO__TEST__TEST_ASSERTS_H__INCLUDED
#define NEUTRINO__TEST__TEST_ASSERTS_H__INCLUDED

// Standard:
#include <cmath>
#include <cstddef>

// Local:
#include "stdexcept.h"


namespace neutrino::test_asserts {

/**
 * Accept any expression without generating 'unused variable' warning.
 * Test expression validity.
 */
template<class T>
	inline void
	verify_compilation (T&&)
	{ }


inline void
verify (std::string const& test_explanation, bool condition)
{
	if (!condition)
		throw TestAssertFailed (test_explanation, "condition failed");
}


template<class T1, class T2, class T3>
	inline void
	verify_equal_with_epsilon (std::string const& test_explanation, T1 const& value1, T2 const& value2, T3 const& epsilon)
	{
		using std::isfinite;

		if (!isfinite (value1) || !isfinite (value2) || value1 - value2 > epsilon || value2 - value1 > epsilon)
		{
			using std::to_string;

			throw TestAssertFailed (test_explanation, "value " + to_string (value1) + " not equal to " +
									to_string (value2) + " with epsilon " +
									to_string (epsilon) + "; diff=" +
									to_string (value2 - value1));
		}
	}

} // namespace neutrino::test_asserts

#endif

