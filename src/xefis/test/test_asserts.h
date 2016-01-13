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

#ifndef XEFIS__TEST__TEST_ASSERTS_H__INCLUDED
#define XEFIS__TEST__TEST_ASSERTS_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "stdexcept.h"


namespace Xefis {
namespace TestAsserts {

inline void
verify (std::string const& explanation, bool condition)
{
	if (!condition)
		throw TestAssertFailed (explanation, "condition failed");
}


template<class T>
	inline void
	verify_equal_with_epsilon (std::string const& explanation, T const& value1, T const& value2, T const& epsilon)
	{
		if (value1 - value2 > epsilon &&
			value2 - value1 > epsilon)
		{
			throw TestAssertFailed (explanation, "value " + boost::lexical_cast<std::string> (value1) + " not equal to " +
									boost::lexical_cast<std::string> (value2) + " with epsilon " +
									boost::lexical_cast<std::string> (epsilon) + "; diff=" +
									boost::lexical_cast<std::string> (value2 - value1));
		}
	}

} // namespace TestAsserts
} // namespace Xefis

#endif

