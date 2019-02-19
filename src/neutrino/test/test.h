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

#ifndef NEUTRINO__TEST__TEST_H__INCLUDED
#define NEUTRINO__TEST__TEST_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>
#include <iostream>
#include <sstream>

// Local:
#include "test_asserts.h"
#include "stdexcept.h"


namespace neutrino {

class RuntimeTest
{
	typedef std::function<void()> TestFunction;

  public:
	RuntimeTest (std::string const& test_name, TestFunction);
};


inline
RuntimeTest::RuntimeTest (std::string const& test_name, TestFunction tf)
{
	static constexpr char kResetColor[]			= "\033[31;1;0m";
	static constexpr char kPassColor[]			= "\033[38;2;100;255;100m";
	static constexpr char kFailColor[]			= "\033[38;2;255;0;0m";
	static constexpr char kExplanationColor[]	= "\033[38;2;225;210;150m";

	std::cout << "Test: " << test_name << "…" << std::flush;
	std::ostringstream log_buffer;

	try {
		tf();
		std::cout << " " << kPassColor << "PASS" << kResetColor << std::endl;
	}
	catch (...)
	{
		std::cout << " " << kFailColor << "FAIL" << kResetColor << std::endl;
		std::cout << kExplanationColor << "Explanation: " << log_buffer.str() << kResetColor;
	}
}

} // namespace neutrino

#endif

