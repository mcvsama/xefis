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

#ifndef XEFIS__CORE__TESTS__TEST_CYCLE_H__INCLUDED
#define XEFIS__CORE__TESTS__TEST_CYCLE_H__INCLUDED

// Xefis:
#include <xefis/core/cycle.h>

// Neutrino:
#include <neutrino/logger.h>

// Standard:
#include <cstddef>
#include <iostream>


namespace xf::test {

/**
 * Just a testing tool.
 */
class TestCycle: public Cycle
{
  public:
	explicit
	TestCycle():
		Cycle (1, 0_s, 1_s, 1_s, s_null_logger)
	{ }

	TestCycle&
	operator+= (si::Time dt)
	{
		Cycle::operator= (Cycle (number() + 1, update_time() + dt, dt, dt, s_null_logger));
		return *this;
	}

  private:
	static inline xf::LoggerOutput	s_logger_output	{ std::clog };
	static inline xf::Logger		s_null_logger	{ s_logger_output };
};

} // namespace xf::test

#endif

