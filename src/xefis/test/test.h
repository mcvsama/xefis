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

#ifndef XEFIS__TEST__TEST_H__INCLUDED
#define XEFIS__TEST__TEST_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>
#include <iostream>
#include <sstream>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/logger.h>

// Local:
#include "test_asserts.h"
#include "stdexcept.h"


namespace xf {

class RuntimeTest
{
	typedef std::function<void()> TestFunction;

  public:
	RuntimeTest (std::string const& test_name, TestFunction);
};


inline
RuntimeTest::RuntimeTest (std::string const& test_name, TestFunction tf)
{
	std::cout << "Test: " << test_name << "…" << std::flush;
	std::ostringstream log_buffer;
	LoggerOutput logger_output (log_buffer);
	logger_output.set_timestamps_enabled (false);
	Logger logger (logger_output);

	bool was_exception = Exception::catch_and_log (logger, [&]{
		tf();
		std::cout << " PASS" << std::endl;
	});

	if (was_exception)
	{
		std::cout << " FAIL" << std::endl;
		std::cout << "Explanation: " << log_buffer.str();
	}
}

} // namespace xf

#endif

